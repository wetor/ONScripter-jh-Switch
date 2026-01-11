/* -*- C++ -*-
 *
 *  test_image_processing.cpp - Image processing tests for ONScripter-jh-Switch
 *
 *  Copyright (C) 2025 ONScripter-jh-Switch contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "test_framework.h"
#include <cstring>
#include <cstdint>
#include <cmath>
#include <algorithm>

//==============================================================================
// Pixel Format and Color Manipulation Helpers
//==============================================================================

// RGBA8888 format helpers
#define RGBA_R(c) ((uint32_t)(((c) >> 24) & 0xFF))
#define RGBA_G(c) ((uint32_t)(((c) >> 16) & 0xFF))
#define RGBA_B(c) ((uint32_t)(((c) >> 8) & 0xFF))
#define RGBA_A(c) ((uint32_t)((c) & 0xFF))
#define MAKE_RGBA(r, g, b, a) ((uint32_t)(((uint32_t)(r) << 24) | ((uint32_t)(g) << 16) | ((uint32_t)(b) << 8) | (uint32_t)(a)))

// Alternative format (ARGB)
#define ARGB_A(c) ((uint32_t)(((c) >> 24) & 0xFF))
#define ARGB_R(c) ((uint32_t)(((c) >> 16) & 0xFF))
#define ARGB_G(c) ((uint32_t)(((c) >> 8) & 0xFF))
#define ARGB_B(c) ((uint32_t)((c) & 0xFF))
#define MAKE_ARGB(a, r, g, b) ((uint32_t)(((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b)))

//==============================================================================
// Simple Bilinear Interpolation for Testing
//==============================================================================

uint32_t bilinearSample(const uint32_t* src, int srcW, int srcH, float u, float v) {
    // Clamp coordinates
    u = std::max(0.0f, std::min(u, (float)(srcW - 1)));
    v = std::max(0.0f, std::min(v, (float)(srcH - 1)));

    int x0 = (int)u;
    int y0 = (int)v;
    int x1 = std::min(x0 + 1, srcW - 1);
    int y1 = std::min(y0 + 1, srcH - 1);

    float fx = u - x0;
    float fy = v - y0;

    uint32_t c00 = src[y0 * srcW + x0];
    uint32_t c10 = src[y0 * srcW + x1];
    uint32_t c01 = src[y1 * srcW + x0];
    uint32_t c11 = src[y1 * srcW + x1];

    // Interpolate each channel
    auto lerp = [](int a, int b, float t) -> int {
        return (int)(a + t * (b - a) + 0.5f);
    };

    int r = lerp(
        lerp(ARGB_R(c00), ARGB_R(c10), fx),
        lerp(ARGB_R(c01), ARGB_R(c11), fx),
        fy
    );
    int g = lerp(
        lerp(ARGB_G(c00), ARGB_G(c10), fx),
        lerp(ARGB_G(c01), ARGB_G(c11), fx),
        fy
    );
    int b = lerp(
        lerp(ARGB_B(c00), ARGB_B(c10), fx),
        lerp(ARGB_B(c01), ARGB_B(c11), fx),
        fy
    );
    int a = lerp(
        lerp(ARGB_A(c00), ARGB_A(c10), fx),
        lerp(ARGB_A(c01), ARGB_A(c11), fx),
        fy
    );

    return MAKE_ARGB(a, r, g, b);
}

//==============================================================================
// Mock Image Resize Function (simplified version of resize_image.cpp logic)
//==============================================================================

void mockResizeImage(
    unsigned char* dst_buffer, int dst_width, int dst_height, int dst_total_width,
    unsigned char* src_buffer, int src_width, int src_height, int src_total_width,
    int byte_per_pixel
) {
    if (dst_width == 0 || dst_height == 0) return;

    int dh1 = dst_height - 1; if (dh1 == 0) dh1 = 1;
    int dw1 = dst_width - 1;  if (dw1 == 0) dw1 = 1;

    for (int i = 0; i < dst_height; i++) {
        int y = (i << 3) * (src_height - 1) / dh1;
        int dy = y & 0x7;
        y >>= 3;

        for (int j = 0; j < dst_width; j++) {
            int x = (j << 3) * (src_width - 1) / dw1;
            int dx = x & 0x7;
            x >>= 3;

            int k = src_total_width * y + x * byte_per_pixel;
            int dst_k = dst_total_width * i + j * byte_per_pixel;

            // Boundary handling
            int mx = (src_width > 1) ? 1 : 0;
            int my = (src_height > 1) ? 1 : 0;

            for (int s = 0; s < byte_per_pixel; s++) {
                unsigned int p;
                p =  (8 - dx) * (8 - dy) * src_buffer[k + s];
                p +=      dx  * (8 - dy) * src_buffer[k + mx * byte_per_pixel + s];
                p += (8 - dx) *      dy  * src_buffer[k + my * src_total_width + s];
                p +=      dx  *      dy  * src_buffer[k + mx * byte_per_pixel + my * src_total_width + s];
                dst_buffer[dst_k + s] = (unsigned char)(p >> 6);
            }
        }
    }

    // Preserve corner pixels
    for (int i = 0; i < byte_per_pixel; i++) {
        dst_buffer[i] = src_buffer[i];
        dst_buffer[(dst_width - 1) * byte_per_pixel + i] =
            src_buffer[(src_width - 1) * byte_per_pixel + i];
        dst_buffer[(dst_height - 1) * dst_total_width + i] =
            src_buffer[(src_height - 1) * src_total_width + i];
        dst_buffer[(dst_height - 1) * dst_total_width + (dst_width - 1) * byte_per_pixel + i] =
            src_buffer[(src_height - 1) * src_total_width + (src_width - 1) * byte_per_pixel + i];
    }
}

//==============================================================================
// Alpha Blending Functions
//==============================================================================

uint32_t alphaBlend(uint32_t src, uint32_t dst, uint8_t alpha) {
    int sr = ARGB_R(src), sg = ARGB_G(src), sb = ARGB_B(src);
    int dr = ARGB_R(dst), dg = ARGB_G(dst), db = ARGB_B(dst);
    int da = ARGB_A(dst);

    int r = (sr * alpha + dr * (255 - alpha)) / 255;
    int g = (sg * alpha + dg * (255 - alpha)) / 255;
    int b = (sb * alpha + db * (255 - alpha)) / 255;

    return MAKE_ARGB(da, r, g, b);
}

uint32_t alphaBlendWithSrcAlpha(uint32_t src, uint32_t dst) {
    int alpha = ARGB_A(src);
    return alphaBlend(src, dst, alpha);
}

// Additive blending (for effects)
uint32_t additiveBlend(uint32_t src, uint32_t dst) {
    int r = std::min(255, (int)(ARGB_R(src) + ARGB_R(dst)));
    int g = std::min(255, (int)(ARGB_G(src) + ARGB_G(dst)));
    int b = std::min(255, (int)(ARGB_B(src) + ARGB_B(dst)));
    int a = ARGB_A(dst);

    return MAKE_ARGB(a, r, g, b);
}

// Subtractive blending
uint32_t subtractiveBlend(uint32_t src, uint32_t dst) {
    int r = std::max(0, (int)ARGB_R(dst) - (int)ARGB_R(src));
    int g = std::max(0, (int)ARGB_G(dst) - (int)ARGB_G(src));
    int b = std::max(0, (int)ARGB_B(dst) - (int)ARGB_B(src));
    int a = ARGB_A(dst);

    return MAKE_ARGB(a, r, g, b);
}

// Multiply blending
uint32_t multiplyBlend(uint32_t src, uint32_t dst) {
    int r = (ARGB_R(src) * ARGB_R(dst)) / 255;
    int g = (ARGB_G(src) * ARGB_G(dst)) / 255;
    int b = (ARGB_B(src) * ARGB_B(dst)) / 255;
    int a = ARGB_A(dst);

    return MAKE_ARGB(a, r, g, b);
}

//==============================================================================
// Pixel Format Conversion Tests
//==============================================================================

TEST_CASE(Image_RGBA_Extraction) {
    uint32_t color = MAKE_RGBA(255, 128, 64, 200);

    ASSERT_EQ(255u, RGBA_R(color));
    ASSERT_EQ(128u, RGBA_G(color));
    ASSERT_EQ(64u, RGBA_B(color));
    ASSERT_EQ(200u, RGBA_A(color));

    return true;
}

TEST_CASE(Image_ARGB_Extraction) {
    uint32_t color = MAKE_ARGB(200, 255, 128, 64);

    ASSERT_EQ(200u, ARGB_A(color));
    ASSERT_EQ(255u, ARGB_R(color));
    ASSERT_EQ(128u, ARGB_G(color));
    ASSERT_EQ(64u, ARGB_B(color));

    return true;
}

TEST_CASE(Image_ColorRoundTrip_RGBA) {
    for (uint32_t r = 0; r < 256; r += 51) {
        for (uint32_t g = 0; g < 256; g += 51) {
            for (uint32_t b = 0; b < 256; b += 51) {
                for (uint32_t a = 0; a < 256; a += 51) {
                    uint32_t color = MAKE_RGBA(r, g, b, a);
                    ASSERT_EQ(r, RGBA_R(color));
                    ASSERT_EQ(g, RGBA_G(color));
                    ASSERT_EQ(b, RGBA_B(color));
                    ASSERT_EQ(a, RGBA_A(color));
                }
            }
        }
    }
    return true;
}

TEST_CASE(Image_ColorRoundTrip_ARGB) {
    for (uint32_t a = 0; a < 256; a += 51) {
        for (uint32_t r = 0; r < 256; r += 51) {
            for (uint32_t g = 0; g < 256; g += 51) {
                for (uint32_t b = 0; b < 256; b += 51) {
                    uint32_t color = MAKE_ARGB(a, r, g, b);
                    ASSERT_EQ(a, ARGB_A(color));
                    ASSERT_EQ(r, ARGB_R(color));
                    ASSERT_EQ(g, ARGB_G(color));
                    ASSERT_EQ(b, ARGB_B(color));
                }
            }
        }
    }
    return true;
}

TEST_CASE(Image_White_Black_Transparent) {
    uint32_t white = MAKE_ARGB(255, 255, 255, 255);
    uint32_t black = MAKE_ARGB(255, 0, 0, 0);
    uint32_t transparent = MAKE_ARGB(0, 0, 0, 0);

    ASSERT_EQ(255u, ARGB_A(white));
    ASSERT_EQ(255u, ARGB_R(white));
    ASSERT_EQ(255u, ARGB_A(black));
    ASSERT_EQ(0u, ARGB_R(black));
    ASSERT_EQ(0u, ARGB_A(transparent));

    return true;
}

//==============================================================================
// Alpha Blending Tests
//==============================================================================

TEST_CASE(Image_AlphaBlend_Opaque) {
    uint32_t src = MAKE_ARGB(255, 255, 0, 0);    // Red
    uint32_t dst = MAKE_ARGB(255, 0, 0, 255);    // Blue

    uint32_t result = alphaBlend(src, dst, 255);

    // With alpha=255, result should be source color
    ASSERT_EQ(255, (int)ARGB_R(result));
    ASSERT_EQ(0, (int)ARGB_G(result));
    ASSERT_EQ(0, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_AlphaBlend_Transparent) {
    uint32_t src = MAKE_ARGB(255, 255, 0, 0);    // Red
    uint32_t dst = MAKE_ARGB(255, 0, 0, 255);    // Blue

    uint32_t result = alphaBlend(src, dst, 0);

    // With alpha=0, result should be destination color
    ASSERT_EQ(0, (int)ARGB_R(result));
    ASSERT_EQ(0, (int)ARGB_G(result));
    ASSERT_EQ(255, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_AlphaBlend_HalfTransparent) {
    uint32_t src = MAKE_ARGB(255, 200, 100, 50);
    uint32_t dst = MAKE_ARGB(255, 100, 200, 150);

    uint32_t result = alphaBlend(src, dst, 128);

    // Should be approximately the average
    uint32_t expected_r = (200 * 128 + 100 * 127) / 255;
    uint32_t expected_g = (100 * 128 + 200 * 127) / 255;
    uint32_t expected_b = (50 * 128 + 150 * 127) / 255;

    ASSERT_NEAR(expected_r, ARGB_R(result), 2u);
    ASSERT_NEAR(expected_g, ARGB_G(result), 2u);
    ASSERT_NEAR(expected_b, ARGB_B(result), 2u);

    return true;
}

TEST_CASE(Image_AlphaBlend_WithSourceAlpha) {
    uint32_t src_opaque = MAKE_ARGB(255, 255, 0, 0);
    uint32_t src_half = MAKE_ARGB(128, 255, 0, 0);
    uint32_t src_transparent = MAKE_ARGB(0, 255, 0, 0);
    uint32_t dst = MAKE_ARGB(255, 0, 0, 255);

    uint32_t result_opaque = alphaBlendWithSrcAlpha(src_opaque, dst);
    uint32_t result_half = alphaBlendWithSrcAlpha(src_half, dst);
    uint32_t result_transparent = alphaBlendWithSrcAlpha(src_transparent, dst);

    // Opaque should show source
    ASSERT_EQ(255, (int)ARGB_R(result_opaque));
    ASSERT_EQ(0, (int)ARGB_B(result_opaque));

    // Transparent should show destination
    ASSERT_EQ(0, (int)ARGB_R(result_transparent));
    ASSERT_EQ(255, (int)ARGB_B(result_transparent));

    // Half should be in between
    ASSERT_GT((int)ARGB_R(result_half), 100);
    ASSERT_LT((int)ARGB_R(result_half), 200);

    return true;
}

TEST_CASE(Image_AdditiveBlend) {
    uint32_t src = MAKE_ARGB(255, 100, 100, 100);
    uint32_t dst = MAKE_ARGB(255, 100, 100, 100);

    uint32_t result = additiveBlend(src, dst);

    ASSERT_EQ(200, (int)ARGB_R(result));
    ASSERT_EQ(200, (int)ARGB_G(result));
    ASSERT_EQ(200, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_AdditiveBlend_Clamped) {
    uint32_t src = MAKE_ARGB(255, 200, 200, 200);
    uint32_t dst = MAKE_ARGB(255, 200, 200, 200);

    uint32_t result = additiveBlend(src, dst);

    // Should clamp to 255, not overflow
    ASSERT_EQ(255, (int)ARGB_R(result));
    ASSERT_EQ(255, (int)ARGB_G(result));
    ASSERT_EQ(255, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_SubtractiveBlend) {
    uint32_t src = MAKE_ARGB(255, 50, 50, 50);
    uint32_t dst = MAKE_ARGB(255, 200, 150, 100);

    uint32_t result = subtractiveBlend(src, dst);

    ASSERT_EQ(150, (int)ARGB_R(result));
    ASSERT_EQ(100, (int)ARGB_G(result));
    ASSERT_EQ(50, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_SubtractiveBlend_Clamped) {
    uint32_t src = MAKE_ARGB(255, 200, 200, 200);
    uint32_t dst = MAKE_ARGB(255, 100, 100, 100);

    uint32_t result = subtractiveBlend(src, dst);

    // Should clamp to 0, not underflow
    ASSERT_EQ(0, (int)ARGB_R(result));
    ASSERT_EQ(0, (int)ARGB_G(result));
    ASSERT_EQ(0, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_MultiplyBlend) {
    uint32_t white = MAKE_ARGB(255, 255, 255, 255);
    uint32_t color = MAKE_ARGB(255, 200, 150, 100);

    uint32_t result = multiplyBlend(white, color);

    // Multiplying by white should preserve the color
    ASSERT_EQ(200, (int)ARGB_R(result));
    ASSERT_EQ(150, (int)ARGB_G(result));
    ASSERT_EQ(100, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_MultiplyBlend_Black) {
    uint32_t black = MAKE_ARGB(255, 0, 0, 0);
    uint32_t color = MAKE_ARGB(255, 200, 150, 100);

    uint32_t result = multiplyBlend(black, color);

    // Multiplying by black should give black
    ASSERT_EQ(0, (int)ARGB_R(result));
    ASSERT_EQ(0, (int)ARGB_G(result));
    ASSERT_EQ(0, (int)ARGB_B(result));

    return true;
}

TEST_CASE(Image_MultiplyBlend_HalfGray) {
    uint32_t gray = MAKE_ARGB(255, 128, 128, 128);
    uint32_t white = MAKE_ARGB(255, 255, 255, 255);

    uint32_t result = multiplyBlend(gray, white);

    // Should be approximately half brightness
    ASSERT_NEAR(128u, ARGB_R(result), 1u);
    ASSERT_NEAR(128u, ARGB_G(result), 1u);
    ASSERT_NEAR(128u, ARGB_B(result), 1u);

    return true;
}

//==============================================================================
// Image Resize Tests
//==============================================================================

TEST_CASE(Image_Resize_SameSize) {
    const int size = 4;
    unsigned char src[size * size * 4];
    unsigned char dst[size * size * 4];

    // Fill source with pattern
    for (int i = 0; i < size * size; i++) {
        src[i * 4 + 0] = i * 16;      // R
        src[i * 4 + 1] = 255 - i * 16; // G
        src[i * 4 + 2] = 128;          // B
        src[i * 4 + 3] = 255;          // A
    }

    mockResizeImage(dst, size, size, size * 4,
                    src, size, size, size * 4, 4);

    // Corners should be preserved
    ASSERT_EQ(src[0], dst[0]);
    ASSERT_EQ(src[1], dst[1]);
    ASSERT_EQ(src[2], dst[2]);
    ASSERT_EQ(src[3], dst[3]);

    return true;
}

TEST_CASE(Image_Resize_Upscale2x) {
    const int srcSize = 2;
    const int dstSize = 4;
    unsigned char src[srcSize * srcSize * 4];
    unsigned char dst[dstSize * dstSize * 4];

    // Create 2x2 image with distinct corners
    // Top-left: Red
    src[0] = 255; src[1] = 0; src[2] = 0; src[3] = 255;
    // Top-right: Green
    src[4] = 0; src[5] = 255; src[6] = 0; src[7] = 255;
    // Bottom-left: Blue
    src[8] = 0; src[9] = 0; src[10] = 255; src[11] = 255;
    // Bottom-right: White
    src[12] = 255; src[13] = 255; src[14] = 255; src[15] = 255;

    mockResizeImage(dst, dstSize, dstSize, dstSize * 4,
                    src, srcSize, srcSize, srcSize * 4, 4);

    // Top-left corner should be red
    ASSERT_EQ(255, dst[0]);
    ASSERT_EQ(0, dst[1]);
    ASSERT_EQ(0, dst[2]);

    // Top-right corner should be green
    int topRightIdx = (dstSize - 1) * 4;
    ASSERT_EQ(0, dst[topRightIdx]);
    ASSERT_EQ(255, dst[topRightIdx + 1]);
    ASSERT_EQ(0, dst[topRightIdx + 2]);

    return true;
}

TEST_CASE(Image_Resize_Downscale) {
    const int srcSize = 8;
    const int dstSize = 4;
    unsigned char src[srcSize * srcSize * 4];
    unsigned char dst[dstSize * dstSize * 4];

    // Fill source with uniform color
    for (int i = 0; i < srcSize * srcSize; i++) {
        src[i * 4 + 0] = 128;
        src[i * 4 + 1] = 128;
        src[i * 4 + 2] = 128;
        src[i * 4 + 3] = 255;
    }

    mockResizeImage(dst, dstSize, dstSize, dstSize * 4,
                    src, srcSize, srcSize, srcSize * 4, 4);

    // Result should be approximately the same color
    for (int i = 0; i < dstSize * dstSize; i++) {
        ASSERT_NEAR(128, (int)dst[i * 4 + 0], 5);
        ASSERT_NEAR(128, (int)dst[i * 4 + 1], 5);
        ASSERT_NEAR(128, (int)dst[i * 4 + 2], 5);
    }

    return true;
}

TEST_CASE(Image_Resize_ZeroSize) {
    unsigned char src[16] = {0};
    unsigned char dst[16] = {0xFF, 0xFF, 0xFF, 0xFF};

    // Should handle zero size gracefully (no crash, no modification)
    mockResizeImage(dst, 0, 0, 0, src, 2, 2, 8, 4);

    // dst should remain unchanged
    ASSERT_EQ(0xFF, dst[0]);

    return true;
}

TEST_CASE(Image_Resize_SinglePixel) {
    unsigned char src[4] = {100, 150, 200, 255};
    unsigned char dst[4] = {0};

    mockResizeImage(dst, 1, 1, 4, src, 1, 1, 4, 4);

    ASSERT_EQ(100, dst[0]);
    ASSERT_EQ(150, dst[1]);
    ASSERT_EQ(200, dst[2]);
    ASSERT_EQ(255, dst[3]);

    return true;
}

//==============================================================================
// Bilinear Sampling Tests
//==============================================================================

TEST_CASE(Image_Bilinear_CornerSampling) {
    uint32_t img[4] = {
        MAKE_ARGB(255, 255, 0, 0),    // Top-left: Red
        MAKE_ARGB(255, 0, 255, 0),    // Top-right: Green
        MAKE_ARGB(255, 0, 0, 255),    // Bottom-left: Blue
        MAKE_ARGB(255, 255, 255, 0),  // Bottom-right: Yellow
    };

    // Sample at exact corners
    uint32_t tl = bilinearSample(img, 2, 2, 0, 0);
    uint32_t tr = bilinearSample(img, 2, 2, 1, 0);
    uint32_t bl = bilinearSample(img, 2, 2, 0, 1);
    uint32_t br = bilinearSample(img, 2, 2, 1, 1);

    ASSERT_EQ(255, (int)ARGB_R(tl));
    ASSERT_EQ(0, (int)ARGB_G(tl));
    ASSERT_EQ(255, (int)ARGB_G(tr));
    ASSERT_EQ(255, (int)ARGB_B(bl));
    ASSERT_EQ(255, (int)ARGB_R(br));
    ASSERT_EQ(255, (int)ARGB_G(br));

    return true;
}

TEST_CASE(Image_Bilinear_CenterSampling) {
    // 2x2 image with corners at different values
    uint32_t img[4] = {
        MAKE_ARGB(255, 0, 0, 0),      // Top-left: Black
        MAKE_ARGB(255, 255, 0, 0),    // Top-right: Red
        MAKE_ARGB(255, 0, 255, 0),    // Bottom-left: Green
        MAKE_ARGB(255, 0, 0, 255),    // Bottom-right: Blue
    };

    // Sample at center (0.5, 0.5)
    uint32_t center = bilinearSample(img, 2, 2, 0.5f, 0.5f);

    // Center should be average of all four
    // R: (0 + 255 + 0 + 0) / 4 ≈ 64
    // G: (0 + 0 + 255 + 0) / 4 ≈ 64
    // B: (0 + 0 + 0 + 255) / 4 ≈ 64
    ASSERT_NEAR(64u, ARGB_R(center), 5u);
    ASSERT_NEAR(64u, ARGB_G(center), 5u);
    ASSERT_NEAR(64u, ARGB_B(center), 5u);

    return true;
}

TEST_CASE(Image_Bilinear_EdgeSampling) {
    uint32_t img[4] = {
        MAKE_ARGB(255, 0, 0, 0),
        MAKE_ARGB(255, 255, 255, 255),
        MAKE_ARGB(255, 0, 0, 0),
        MAKE_ARGB(255, 255, 255, 255),
    };

    // Sample at top edge center
    uint32_t topCenter = bilinearSample(img, 2, 2, 0.5f, 0);

    // Should be average of top-left and top-right
    ASSERT_NEAR(128u, ARGB_R(topCenter), 5u);
    ASSERT_NEAR(128u, ARGB_G(topCenter), 5u);
    ASSERT_NEAR(128u, ARGB_B(topCenter), 5u);

    return true;
}

//==============================================================================
// Clipping and Bounding Tests (from AnimationInfo::doClipping)
//==============================================================================

struct ClipRect {
    int x, y, w, h;
};

int doClipping(ClipRect* dst, ClipRect* clip, ClipRect* clipped) {
    if (clipped) {
        clipped->x = 0;
        clipped->y = 0;
    }

    if (!dst ||
        dst->x >= clip->x + clip->w || dst->x + dst->w <= clip->x ||
        dst->y >= clip->y + clip->h || dst->y + dst->h <= clip->y) {
        return -1;
    }

    if (dst->x < clip->x) {
        dst->w -= clip->x - dst->x;
        if (clipped) clipped->x = clip->x - dst->x;
        dst->x = clip->x;
    }
    if (clip->x + clip->w < dst->x + dst->w) {
        dst->w = clip->x + clip->w - dst->x;
    }

    if (dst->y < clip->y) {
        dst->h -= clip->y - dst->y;
        if (clipped) clipped->y = clip->y - dst->y;
        dst->y = clip->y;
    }
    if (clip->y + clip->h < dst->y + dst->h) {
        dst->h = clip->y + clip->h - dst->y;
    }

    if (clipped) {
        clipped->w = dst->w;
        clipped->h = dst->h;
    }

    return 0;
}

TEST_CASE(Image_Clipping_FullyInside) {
    ClipRect dst = {100, 100, 50, 50};
    ClipRect clip = {0, 0, 640, 480};
    ClipRect clipped;

    int result = doClipping(&dst, &clip, &clipped);

    ASSERT_EQ(0, result);
    ASSERT_EQ(100, dst.x);
    ASSERT_EQ(100, dst.y);
    ASSERT_EQ(50, dst.w);
    ASSERT_EQ(50, dst.h);

    return true;
}

TEST_CASE(Image_Clipping_PartiallyOutsideLeft) {
    ClipRect dst = {-20, 100, 50, 50};
    ClipRect clip = {0, 0, 640, 480};
    ClipRect clipped;

    int result = doClipping(&dst, &clip, &clipped);

    ASSERT_EQ(0, result);
    ASSERT_EQ(0, dst.x);
    ASSERT_EQ(30, dst.w);
    ASSERT_EQ(20, clipped.x);

    return true;
}

TEST_CASE(Image_Clipping_PartiallyOutsideRight) {
    ClipRect dst = {600, 100, 100, 50};
    ClipRect clip = {0, 0, 640, 480};
    ClipRect clipped;

    int result = doClipping(&dst, &clip, &clipped);

    ASSERT_EQ(0, result);
    ASSERT_EQ(600, dst.x);
    ASSERT_EQ(40, dst.w);
    ASSERT_EQ(0, clipped.x);

    return true;
}

TEST_CASE(Image_Clipping_FullyOutside) {
    ClipRect dst = {700, 100, 50, 50};
    ClipRect clip = {0, 0, 640, 480};
    ClipRect clipped;

    int result = doClipping(&dst, &clip, &clipped);

    ASSERT_EQ(-1, result);

    return true;
}

TEST_CASE(Image_Clipping_AllEdges) {
    ClipRect dst = {-10, -10, 700, 500};
    ClipRect clip = {0, 0, 640, 480};
    ClipRect clipped;

    int result = doClipping(&dst, &clip, &clipped);

    ASSERT_EQ(0, result);
    ASSERT_EQ(0, dst.x);
    ASSERT_EQ(0, dst.y);
    ASSERT_EQ(640, dst.w);
    ASSERT_EQ(480, dst.h);
    ASSERT_EQ(10, clipped.x);
    ASSERT_EQ(10, clipped.y);

    return true;
}

//==============================================================================
// Pixel Row Operations (common in image processing)
//==============================================================================

void fillRow(unsigned char* row, int width, unsigned char r, unsigned char g,
             unsigned char b, unsigned char a) {
    for (int i = 0; i < width; i++) {
        row[i * 4 + 0] = r;
        row[i * 4 + 1] = g;
        row[i * 4 + 2] = b;
        row[i * 4 + 3] = a;
    }
}

void copyRow(unsigned char* dst, const unsigned char* src, int width) {
    memcpy(dst, src, width * 4);
}

TEST_CASE(Image_RowFill) {
    unsigned char row[32];  // 8 pixels
    fillRow(row, 8, 255, 128, 64, 200);

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(255, row[i * 4 + 0]);
        ASSERT_EQ(128, row[i * 4 + 1]);
        ASSERT_EQ(64, row[i * 4 + 2]);
        ASSERT_EQ(200, row[i * 4 + 3]);
    }

    return true;
}

TEST_CASE(Image_RowCopy) {
    unsigned char src[32];
    unsigned char dst[32];

    fillRow(src, 8, 100, 150, 200, 250);
    memset(dst, 0, 32);

    copyRow(dst, src, 8);

    for (int i = 0; i < 32; i++) {
        ASSERT_EQ(src[i], dst[i]);
    }

    return true;
}

//==============================================================================
// Grayscale Conversion Tests
//==============================================================================

unsigned char toGrayscale(unsigned char r, unsigned char g, unsigned char b) {
    // Standard luminance formula: 0.299R + 0.587G + 0.114B
    return (unsigned char)((r * 299 + g * 587 + b * 114) / 1000);
}

TEST_CASE(Image_Grayscale_White) {
    unsigned char gray = toGrayscale(255, 255, 255);
    ASSERT_EQ(255, gray);
    return true;
}

TEST_CASE(Image_Grayscale_Black) {
    unsigned char gray = toGrayscale(0, 0, 0);
    ASSERT_EQ(0, gray);
    return true;
}

TEST_CASE(Image_Grayscale_Red) {
    unsigned char gray = toGrayscale(255, 0, 0);
    // 255 * 0.299 ≈ 76
    ASSERT_NEAR(76, gray, 2);
    return true;
}

TEST_CASE(Image_Grayscale_Green) {
    unsigned char gray = toGrayscale(0, 255, 0);
    // 255 * 0.587 ≈ 150
    ASSERT_NEAR(150, gray, 2);
    return true;
}

TEST_CASE(Image_Grayscale_Blue) {
    unsigned char gray = toGrayscale(0, 0, 255);
    // 255 * 0.114 ≈ 29
    ASSERT_NEAR(29, gray, 2);
    return true;
}

//==============================================================================
// Performance-Related Structure Tests
//==============================================================================

// Test structure alignment (important for SIMD operations)
TEST_CASE(Image_StructAlignment) {
    struct alignas(16) AlignedPixel {
        uint32_t data[4];
    };

    AlignedPixel pixels[2];

    // Check 16-byte alignment
    ASSERT_EQ(0u, ((uintptr_t)&pixels[0]) % 16);
    ASSERT_EQ(0u, ((uintptr_t)&pixels[1]) % 16);

    return true;
}

TEST_CASE(Image_RowStride) {
    // Test that row stride calculations are correct
    int width = 640;
    int bpp = 4;
    int stride = width * bpp;

    ASSERT_EQ(2560, stride);

    // With alignment to 16 bytes
    int aligned_stride = ((stride + 15) / 16) * 16;
    ASSERT_EQ(2560, aligned_stride);  // 2560 is already 16-byte aligned

    // Non-aligned case
    width = 641;
    stride = width * bpp;
    aligned_stride = ((stride + 15) / 16) * 16;
    ASSERT_EQ(2576, aligned_stride);  // Rounded up from 2564

    return true;
}

//==============================================================================
// Switch-specific Resolution Scaling Tests
//==============================================================================

TEST_CASE(Image_Switch_DockedScaling) {
    // Switch docked: 1920x1080
    // Original game: 640x480 (4:3)
    // Need to letterbox in 16:9

    int game_w = 640, game_h = 480;
    int screen_w = 1920, screen_h = 1080;

    // Calculate scaled size maintaining aspect ratio
    float game_aspect = (float)game_w / game_h;     // 1.333
    float screen_aspect = (float)screen_w / screen_h; // 1.777

    int scaled_w, scaled_h;
    if (game_aspect < screen_aspect) {
        // Letterbox (bars on sides)
        scaled_h = screen_h;
        scaled_w = (int)(screen_h * game_aspect);
    } else {
        // Pillarbox (bars on top/bottom)
        scaled_w = screen_w;
        scaled_h = (int)(screen_w / game_aspect);
    }

    // 4:3 in 16:9 should be letterboxed
    ASSERT_EQ(1080, scaled_h);
    ASSERT_EQ(1440, scaled_w);

    return true;
}

TEST_CASE(Image_Switch_HandheldScaling) {
    // Switch handheld: 1280x720
    int game_w = 640, game_h = 480;
    int screen_w = 1280, screen_h = 720;

    float game_aspect = (float)game_w / game_h;

    int scaled_h = screen_h;
    int scaled_w = (int)(screen_h * game_aspect);

    ASSERT_EQ(720, scaled_h);
    ASSERT_EQ(960, scaled_w);

    // Calculate offset for centering
    int offset_x = (screen_w - scaled_w) / 2;
    ASSERT_EQ(160, offset_x);

    return true;
}

TEST_CASE(Image_Switch_800x600_Scaling) {
    // Some games use 800x600
    int game_w = 800, game_h = 600;
    int screen_w = 1920, screen_h = 1080;

    float game_aspect = (float)game_w / game_h;  // 1.333

    int scaled_h = screen_h;
    int scaled_w = (int)(screen_h * game_aspect);

    ASSERT_EQ(1080, scaled_h);
    ASSERT_EQ(1440, scaled_w);

    return true;
}
