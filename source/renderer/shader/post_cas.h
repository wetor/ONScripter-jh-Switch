/* -*- C++ -*-
 *
 *  post_cas.h - CAS (Contrast Adaptive Sharpening) shader for GLES2
 *
 *  Based on AMD FidelityFX CAS
 *  Adapted for ONScripter Nintendo Switch port
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

#ifndef POST_CAS_H_
#define POST_CAS_H_

/*
 * CAS (Contrast Adaptive Sharpening) GLSL shader
 * This is a simplified version optimized for GLES 2.0/3.0 on Nintendo Switch
 *
 * The shader applies adaptive sharpening based on local contrast analysis.
 * It samples neighboring pixels and enhances edges while preserving details.
 */

const char *post_cas_glsl =
    "#version 300 es\n"
    "precision highp float;\n"
    "precision highp int;\n"
    "\n"
    "uniform vec4 Const0;\n"  // x: input_w/output_w, y: input_h/output_h, z: 0.5*x-0.5, w: 0.5*y-0.5
    "uniform vec4 Const1;\n"  // x,y: sharpness, z: 8*scale_x, w: input_h
    "uniform vec4 Const2;\n"  // x: window_x, y: window_y, z,w: unused
    "uniform sampler2D u_texture;\n"
    "\n"
    "out vec4 color;\n"
    "\n"
    "// Utility functions\n"
    "float saturate(float x) { return clamp(x, 0.0, 1.0); }\n"
    "vec3 saturate3(vec3 x) { return clamp(x, vec3(0.0), vec3(1.0)); }\n"
    "\n"
    "// Fast approximate reciprocal\n"
    "float aprxRcp(float x) { return 1.0 / x; }\n"
    "float aprxRcpSqrt(float x) { return inversesqrt(x); }\n"
    "\n"
    "// Min/Max helpers\n"
    "float min3(float a, float b, float c) { return min(a, min(b, c)); }\n"
    "float max3(float a, float b, float c) { return max(a, max(b, c)); }\n"
    "\n"
    "// Load texel with Y-flip for correct orientation\n"
    "vec3 CasLoad(ivec2 p) {\n"
    "    // Flip Y coordinate and sample BGR->RGB\n"
    "    return texelFetch(u_texture, ivec2(p.x, int(Const1.w) - p.y), 0).zyx;\n"
    "}\n"
    "\n"
    "// CAS filter implementation\n"
    "void CasFilter(out float pixR, out float pixG, out float pixB, uvec2 ip, bool noScaling) {\n"
    "    ivec2 sp = ivec2(ip);\n"
    "    \n"
    "    // Sample 3x3 neighborhood\n"
    "    vec3 a = CasLoad(sp + ivec2(-1, -1));\n"
    "    vec3 b = CasLoad(sp + ivec2( 0, -1));\n"
    "    vec3 c = CasLoad(sp + ivec2( 1, -1));\n"
    "    vec3 d = CasLoad(sp + ivec2(-1,  0));\n"
    "    vec3 e = CasLoad(sp + ivec2( 0,  0));\n"
    "    vec3 f = CasLoad(sp + ivec2( 1,  0));\n"
    "    vec3 g = CasLoad(sp + ivec2(-1,  1));\n"
    "    vec3 h = CasLoad(sp + ivec2( 0,  1));\n"
    "    vec3 i = CasLoad(sp + ivec2( 1,  1));\n"
    "    \n"
    "    // Soft min and max (plus shape)\n"
    "    //  b\n"
    "    // d e f\n"
    "    //  h\n"
    "    float mnR = min3(min3(d.r, e.r, f.r), b.r, h.r);\n"
    "    float mnG = min3(min3(d.g, e.g, f.g), b.g, h.g);\n"
    "    float mnB = min3(min3(d.b, e.b, f.b), b.b, h.b);\n"
    "    \n"
    "    // Add diagonals for better quality\n"
    "    float mnR2 = min3(min3(mnR, a.r, c.r), g.r, i.r);\n"
    "    float mnG2 = min3(min3(mnG, a.g, c.g), g.g, i.g);\n"
    "    float mnB2 = min3(min3(mnB, a.b, c.b), g.b, i.b);\n"
    "    mnR = mnR + mnR2;\n"
    "    mnG = mnG + mnG2;\n"
    "    mnB = mnB + mnB2;\n"
    "    \n"
    "    float mxR = max3(max3(d.r, e.r, f.r), b.r, h.r);\n"
    "    float mxG = max3(max3(d.g, e.g, f.g), b.g, h.g);\n"
    "    float mxB = max3(max3(d.b, e.b, f.b), b.b, h.b);\n"
    "    \n"
    "    float mxR2 = max3(max3(mxR, a.r, c.r), g.r, i.r);\n"
    "    float mxG2 = max3(max3(mxG, a.g, c.g), g.g, i.g);\n"
    "    float mxB2 = max3(max3(mxB, a.b, c.b), g.b, i.b);\n"
    "    mxR = mxR + mxR2;\n"
    "    mxG = mxG + mxG2;\n"
    "    mxB = mxB + mxB2;\n"
    "    \n"
    "    // Smooth minimum distance to signal limit divided by smooth max\n"
    "    float rcpMR = aprxRcp(mxR);\n"
    "    float rcpMG = aprxRcp(mxG);\n"
    "    float rcpMB = aprxRcp(mxB);\n"
    "    \n"
    "    // Shaping amount of sharpening\n"
    "    float ampR = saturate(min(mnR, 2.0 - mxR) * rcpMR);\n"
    "    float ampG = saturate(min(mnG, 2.0 - mxG) * rcpMG);\n"
    "    float ampB = saturate(min(mnB, 2.0 - mxB) * rcpMB);\n"
    "    \n"
    "    // Square to increase quality\n"
    "    ampR = sqrt(ampR);\n"
    "    ampG = sqrt(ampG);\n"
    "    ampB = sqrt(ampB);\n"
    "    \n"
    "    // Filter shape:\n"
    "    //  0 w 0\n"
    "    //  w 1 w\n"
    "    //  0 w 0\n"
    "    float peak = Const1.x;\n"
    "    float wG = ampG * peak;\n"
    "    \n"
    "    // Use green channel weight for all (faster, usually looks fine)\n"
    "    float rcpWeight = aprxRcp(1.0 + 4.0 * wG);\n"
    "    \n"
    "    pixR = saturate((b.r * wG + d.r * wG + f.r * wG + h.r * wG + e.r) * rcpWeight);\n"
    "    pixG = saturate((b.g * wG + d.g * wG + f.g * wG + h.g * wG + e.g) * rcpWeight);\n"
    "    pixB = saturate((b.b * wG + d.b * wG + f.b * wG + h.b * wG + e.b) * rcpWeight);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec3 c;\n"
    "    // Apply offset from Const2 (window position)\n"
    "    uvec2 fragPos = uvec2(gl_FragCoord.xy + vec2(-Const2.x, Const2.y));\n"
    "    CasFilter(c.r, c.g, c.b, fragPos, false);\n"
    "    color = vec4(c, 1.0);\n"
    "}\n";

#endif // POST_CAS_H_
