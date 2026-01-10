/* -*- C++ -*-
 *
 *  gles_renderer.cpp - GLES2 renderer for ONScripter (Nintendo Switch port)
 *
 *  Copyright (C) 2022 jh10001 <jh10001@live.cn>
 *  Copyright (C) 2023-2024 yurisizuku <https://github.com/YuriSizuku>
 *  Copyright (C) 2024 Nintendo Switch port adaptation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined(USE_GLES)

#include "gles_renderer.h"
#include "Utils.h"
#include "shader/post_cas.h"

// Error checking macro
#define GLES_CHECK_ERROR(tag) \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            utils::printError("%s: GLES error: (0x%X)\n", tag, err); \
        } \
    } while (0)

// Vertex shader source - simple passthrough
static const GLchar *post_vert_src = R"GLSL(#version 300 es
in vec2 a_position;
void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
}
)GLSL";

/**
 * Setup CAS (Contrast Adaptive Sharpening) constants
 *
 * @param con Output array of 8 floats for shader uniforms
 * @param sharpness Sharpening strength (0.0 to 1.0)
 * @param inputSizeInPixels Input texture dimensions
 * @param outputSizeInPixels Output render dimensions
 */
static void casSetup(GLfloat con[4*2], float sharpness,
                     const float inputSizeInPixels[2],
                     const float outputSizeInPixels[2])
{
    // Scaling terms
    con[0] = inputSizeInPixels[0] * (1.0f / outputSizeInPixels[0]);
    con[1] = inputSizeInPixels[1] * (1.0f / outputSizeInPixels[1]);
    con[2] = 0.5f * inputSizeInPixels[0] * (1.0f / outputSizeInPixels[0]) - 0.5f;
    con[3] = 0.5f * inputSizeInPixels[1] * (1.0f / outputSizeInPixels[1]) - 0.5f;

    // Sharpness value - convert from 0-1 range to shader parameter
    // Higher sharpness value = more sharpening
    const float sharp = 1.0f / (8.0f - sharpness * 3.0f);
    con[4] = sharp;
    con[5] = sharp;
    con[6] = 8.0f * inputSizeInPixels[0] * (1.0f / outputSizeInPixels[0]);
    con[7] = inputSizeInPixels[1];
}

void GlesRenderer::setConstBuffer(const float input_size[2],
                                   const float output_size[2],
                                   const float sharpness)
{
    this->output_size[0] = static_cast<int>(output_size[0]);
    this->output_size[1] = static_cast<int>(output_size[1]);
    casSetup(cas_con, sharpness, input_size, output_size);
}

void GlesRenderer::initVertexData()
{
    // Setup fullscreen quad vertices in NDC (-1 to 1)
    GLfloat minu = 0.0f, maxu = 1.0f, minv = 0.0f, maxv = 1.0f;

    // Triangle strip: bottom-left, bottom-right, top-left, top-right
    vertex_data[0] = minu; vertex_data[1] = maxv;  // BL
    vertex_data[2] = maxu; vertex_data[3] = maxv;  // BR
    vertex_data[4] = minu; vertex_data[5] = minv;  // TL
    vertex_data[6] = maxu; vertex_data[7] = minv;  // TR

    // Convert from 0-1 to -1 to 1 (NDC)
    for (int i = 0; i < 8; i++) {
        vertex_data[i] = vertex_data[i] * 2.0f - 1.0f;
    }

    // Create and upload vertex buffer
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    GLES_CHECK_ERROR("initVertexData");
}

GlesRenderer::GlesRenderer(SDL_Window *window, SDL_Texture *texture,
                           const float input_size[2], const float output_size[2],
                           const float sharpness)
    : window(window)
    , texture(texture)
    , context(nullptr)
    , vert_shader(0)
    , frag_shader(0)
    , post_program(0)
    , vertex_buffer(0)
    , _pause(false)
{
    // Initialize arrays
    for (int i = 0; i < 8; i++) {
        vertex_data[i] = 0.0f;
        cas_con[i] = 0.0f;
    }
    for (int i = 0; i < 3; i++) {
        const_buffer_location[i] = -1;
    }
    this->output_size[0] = 0;
    this->output_size[1] = 0;

    // Bind SDL texture to get OpenGL texture handle
    SDL_GL_BindTexture(texture, nullptr, nullptr);

    // Get current GL context
    context = SDL_GL_GetCurrentContext();
    if (!context) {
        utils::printError("GlesRenderer: Failed to get GL context: %s\n", SDL_GetError());
        return;
    }

    // Create shaders
    vert_shader = createShader(GL_VERTEX_SHADER, post_vert_src);
    if (vert_shader == 0) {
        utils::printError("GlesRenderer: Failed to create vertex shader\n");
        return;
    }

    frag_shader = createShader(GL_FRAGMENT_SHADER, post_cas_glsl);
    if (frag_shader == 0) {
        utils::printError("GlesRenderer: Failed to create fragment shader\n");
        glDeleteShader(vert_shader);
        vert_shader = 0;
        return;
    }

    // Create and link program
    post_program = glCreateProgram();
    if (post_program == 0) {
        utils::printError("GlesRenderer: Failed to create program\n");
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        vert_shader = frag_shader = 0;
        return;
    }

    glAttachShader(post_program, vert_shader);
    glAttachShader(post_program, frag_shader);

    // Bind attribute locations before linking
    glBindAttribLocation(post_program, 0, "a_position");
    glBindAttribLocation(post_program, 1, "a_texCoord");

    glLinkProgram(post_program);

    // Check link status
    GLint link_status = 0;
    glGetProgramiv(post_program, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        char info_log[512];
        glGetProgramInfoLog(post_program, sizeof(info_log), nullptr, info_log);
        utils::printError("GlesRenderer: Failed to link program:\n%s\n", info_log);
        glDeleteProgram(post_program);
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        post_program = vert_shader = frag_shader = 0;
        return;
    }

    // Get uniform locations
    const_buffer_location[0] = glGetUniformLocation(post_program, "Const0");
    const_buffer_location[1] = glGetUniformLocation(post_program, "Const1");
    const_buffer_location[2] = glGetUniformLocation(post_program, "Const2");

    // Setup constants and vertex data
    setConstBuffer(input_size, output_size, sharpness);
    initVertexData();

    utils::printInfo("GlesRenderer: Initialized successfully (sharpness=%.2f)\n", sharpness);
    GLES_CHECK_ERROR("GlesRenderer constructor");
}

GlesRenderer::~GlesRenderer()
{
    if (vertex_buffer != 0) {
        glDeleteBuffers(1, &vertex_buffer);
        vertex_buffer = 0;
    }
    if (post_program != 0) {
        glDeleteProgram(post_program);
        post_program = 0;
    }
    if (vert_shader != 0) {
        glDeleteShader(vert_shader);
        vert_shader = 0;
    }
    if (frag_shader != 0) {
        glDeleteShader(frag_shader);
        frag_shader = 0;
    }
}

GLuint GlesRenderer::createShader(const GLenum shader_type, const GLchar *shader_src)
{
    GLuint shader = glCreateShader(shader_type);
    if (shader == 0) {
        utils::printError("GlesRenderer: glCreateShader failed\n");
        return 0;
    }

    glShaderSource(shader, 1, &shader_src, nullptr);
    glCompileShader(shader);

    // Check compile status
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char info_log[512];
        glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);
        utils::printError("GlesRenderer: Shader compile error:\n%s\n", info_log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void GlesRenderer::copy(const int window_x, const int window_y)
{
    if (_pause) {
        return;
    }

    // Ensure we're using the correct GL context
    if (SDL_GL_GetCurrentContext() != context) {
        if (SDL_GL_MakeCurrent(window, context) < 0) {
            utils::printError("GlesRenderer::copy: Failed to make context current\n");
            return;
        }
    }

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    SDL_GL_BindTexture(texture, nullptr, nullptr);

    // Set viewport - note: window_y is negated for coordinate system conversion
    glViewport(window_x, -window_y, output_size[0], output_size[1]);

    // Setup vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    // Use our CAS shader program
    glUseProgram(post_program);

    // Setup vertex attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Set shader uniforms
    glUniform4f(const_buffer_location[0],
                cas_con[0], cas_con[1], cas_con[2], cas_con[3]);
    glUniform4f(const_buffer_location[1],
                cas_con[4], cas_con[5], cas_con[6], cas_con[7]);
    glUniform4f(const_buffer_location[2],
                static_cast<float>(window_x), static_cast<float>(window_y),
                0.0f, 0.0f);

    // Draw fullscreen quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    GLES_CHECK_ERROR("GlesRenderer::copy");
}

void GlesRenderer::pause()
{
    _pause = true;
}

void GlesRenderer::resume()
{
    _pause = false;
}

#endif // USE_GLES
