/*
 * software RGB to RGB converter
 * pluralize by software PAL8 to RGB converter
 *              software YUV to YUV converter
 *              software YUV to RGB converter
 * Written by Nick Kurshev.
 * palette & YUV & runtime CPU stuff by Michael (michaelni@gmx.at)
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <inttypes.h>
#include "config.h"
#include "libavutil/bswap.h"
#include "rgb2rgb.h"
#include "swscale.h"
#include "swscale_internal.h"

void (*rgb24tobgr32)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24tobgr16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24tobgr15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32to16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32to15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb15to16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb15tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb15to32)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb16to15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb16tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb16to32)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24to16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24to15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*shuffle_bytes_2103)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32tobgr16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32tobgr15)(const uint8_t *src, uint8_t *dst, int src_size);

void (*yv12toyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                   int width, int height,
                   int lumStride, int chromStride, int dstStride);
void (*yv12touyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                   int width, int height,
                   int lumStride, int chromStride, int dstStride);
void (*yuv422ptoyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                      int width, int height,
                      int lumStride, int chromStride, int dstStride);
void (*yuv422ptouyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                      int width, int height,
                      int lumStride, int chromStride, int dstStride);
void (*yuy2toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                   int width, int height,
                   int lumStride, int chromStride, int srcStride);
void (*rgb24toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                    int width, int height,
                    int lumStride, int chromStride, int srcStride);
void (*planar2x)(const uint8_t *src, uint8_t *dst, int width, int height,
                 int srcStride, int dstStride);
void (*interleaveBytes)(const uint8_t *src1, const uint8_t *src2, uint8_t *dst,
                        int width, int height, int src1Stride,
                        int src2Stride, int dstStride);
void (*vu9_to_vu12)(const uint8_t *src1, const uint8_t *src2,
                    uint8_t *dst1, uint8_t *dst2,
                    int width, int height,
                    int srcStride1, int srcStride2,
                    int dstStride1, int dstStride2);
void (*yvu9_to_yuy2)(const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
                     uint8_t *dst,
                     int width, int height,
                     int srcStride1, int srcStride2,
                     int srcStride3, int dstStride);
void (*uyvytoyuv420)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                     int width, int height,
                     int lumStride, int chromStride, int srcStride);
void (*uyvytoyuv422)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                     int width, int height,
                     int lumStride, int chromStride, int srcStride);
void (*yuyvtoyuv420)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                     int width, int height,
                     int lumStride, int chromStride, int srcStride);
void (*yuyvtoyuv422)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                     int width, int height,
                     int lumStride, int chromStride, int srcStride);

#define RGB2YUV_SHIFT 8
#define BY ((int)( 0.098*(1<<RGB2YUV_SHIFT)+0.5))
#define BV ((int)(-0.071*(1<<RGB2YUV_SHIFT)+0.5))
#define BU ((int)( 0.439*(1<<RGB2YUV_SHIFT)+0.5))
#define GY ((int)( 0.504*(1<<RGB2YUV_SHIFT)+0.5))
#define GV ((int)(-0.368*(1<<RGB2YUV_SHIFT)+0.5))
#define GU ((int)(-0.291*(1<<RGB2YUV_SHIFT)+0.5))
#define RY ((int)( 0.257*(1<<RGB2YUV_SHIFT)+0.5))
#define RV ((int)( 0.439*(1<<RGB2YUV_SHIFT)+0.5))
#define RU ((int)(-0.148*(1<<RGB2YUV_SHIFT)+0.5))

//plain C versions
#include "rgb2rgb_template.c"


/*
 RGB15->RGB16 original by Strepto/Astral
 ported to gcc & bugfixed : A'rpi
 MMX2, 3DNOW optimization by Nick Kurshev
 32-bit C version, and and&add trick by Michael Niedermayer
*/

void sws_rgb2rgb_init(void)
{
    rgb2rgb_init_c();
    if (HAVE_MMX)
        rgb2rgb_init_x86();
}

void rgb32to24(const uint8_t *src, uint8_t *dst, int src_size)
{
    int i, num_pixels = src_size >> 2;

    for (i=0; i<num_pixels; i++) {
#if HAVE_BIGENDIAN
        /* RGB32 (= A,B,G,R) -> BGR24 (= B,G,R) */
        dst[3*i + 0] = src[4*i + 1];
        dst[3*i + 1] = src[4*i + 2];
        dst[3*i + 2] = src[4*i + 3];
#else
        dst[3*i + 0] = src[4*i + 2];
        dst[3*i + 1] = src[4*i + 1];
        dst[3*i + 2] = src[4*i + 0];
#endif
    }
}

void rgb24to32(const uint8_t *src, uint8_t *dst, int src_size)
{
    int i;
    for (i=0; 3*i<src_size; i++) {
#if HAVE_BIGENDIAN
        /* RGB24 (= R,G,B) -> BGR32 (= A,R,G,B) */
        dst[4*i + 0] = 255;
        dst[4*i + 1] = src[3*i + 0];
        dst[4*i + 2] = src[3*i + 1];
        dst[4*i + 3] = src[3*i + 2];
#else
        dst[4*i + 0] = src[3*i + 2];
        dst[4*i + 1] = src[3*i + 1];
        dst[4*i + 2] = src[3*i + 0];
        dst[4*i + 3] = 255;
#endif
    }
}

void rgb16tobgr32(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    const uint16_t *end = s + src_size / 2;

    while (s < end) {
        register uint16_t bgr = *s++;
#if HAVE_BIGENDIAN
        *d++ = 255;
        *d++ = (bgr&0x1F)<<3;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0xF800)>>8;
#else
        *d++ = (bgr&0xF800)>>8;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0x1F)<<3;
        *d++ = 255;
#endif
    }
}

void rgb12to15(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    const uint16_t *s = (const uint16_t *)src;
    uint16_t rgb, r, g, b;
    const uint16_t *end = s + src_size / 2;

    while (s < end) {
        rgb = *s++;
        r = rgb & 0xF00;
        g = rgb & 0x0F0;
        b = rgb & 0x00F;
        r = (r << 3) | ((r & 0x800) >> 1);
        g = (g << 2) | ((g & 0x080) >> 2);
        b = (b << 1) | ( b          >> 3);
        *d++ = r | g | b;
    }
}

void rgb16to24(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    const uint16_t *end = s + src_size / 2;

    while (s < end) {
        register uint16_t bgr = *s++;
        *d++ = (bgr&0xF800)>>8;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb16tobgr16(const uint8_t *src, uint8_t *dst, int src_size)
{
    int i, num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++) {
        unsigned rgb = ((const uint16_t*)src)[i];
        ((uint16_t*)dst)[i] = (rgb>>11) | (rgb&0x7E0) | (rgb<<11);
    }
}

void rgb16tobgr15(const uint8_t *src, uint8_t *dst, int src_size)
{
    int i, num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++) {
        unsigned rgb = ((const uint16_t*)src)[i];
        ((uint16_t*)dst)[i] = (rgb>>11) | ((rgb&0x7C0)>>1) | ((rgb&0x1F)<<10);
    }
}

void rgb15tobgr32(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    const uint16_t *end = s + src_size / 2;

    while (s < end) {
        register uint16_t bgr = *s++;
#if HAVE_BIGENDIAN
        *d++ = 255;
        *d++ = (bgr&0x1F)<<3;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x7C00)>>7;
#else
        *d++ = (bgr&0x7C00)>>7;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x1F)<<3;
        *d++ = 255;
#endif
    }
}

void rgb15to24(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    const uint16_t *end = s + src_size / 2;

    while (s < end) {
        register uint16_t bgr = *s++;
        *d++ = (bgr&0x7C00)>>7;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb15tobgr16(const uint8_t *src, uint8_t *dst, int src_size)
{
    int i, num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++) {
        unsigned rgb = ((const uint16_t*)src)[i];
        ((uint16_t*)dst)[i] = ((rgb&0x7C00)>>10) | ((rgb&0x3E0)<<1) | (rgb<<11);
    }
}

void rgb15tobgr15(const uint8_t *src, uint8_t *dst, int src_size)
{
    int i, num_pixels = src_size >> 1;

    for (i=0; i<num_pixels; i++) {
        unsigned rgb = ((const uint16_t*)src)[i];
        unsigned br  = rgb & 0x7C1F;
        ((uint16_t*)dst)[i] = (br>>10) | (rgb&0x3E0) | (br<<10);
    }
}

void rgb12tobgr12(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t*)dst;
    uint16_t *s = (uint16_t*)src;
    int i, num_pixels = src_size >> 1;

    for (i = 0; i < num_pixels; i++) {
        unsigned rgb = s[i];
        d[i] = (rgb << 8 | rgb & 0xF0 | rgb >> 8) & 0xFFF;
    }
}

void bgr8torgb8(const uint8_t *src, uint8_t *dst, int src_size)
{
    int i, num_pixels = src_size;

    for (i=0; i<num_pixels; i++) {
        register uint8_t rgb = src[i];
        unsigned r           = (rgb & 0x07);
        unsigned g           = (rgb & 0x38) >> 3;
        unsigned b           = (rgb & 0xC0) >> 6;
        dst[i] = ((b<<1)&0x07) | ((g&0x07)<<3) | ((r&0x03)<<6);
    }
}

#define DEFINE_SHUFFLE_BYTES(a, b, c, d)                                \
void shuffle_bytes_##a##b##c##d(const uint8_t *src, uint8_t *dst, int src_size) \
{                                                                       \
    int i;                                                             \
                                                                        \
    for (i = 0; i < src_size; i+=4) {                                   \
        dst[i + 0] = src[i + a];                                        \
        dst[i + 1] = src[i + b];                                        \
        dst[i + 2] = src[i + c];                                        \
        dst[i + 3] = src[i + d];                                        \
    }                                                                   \
}

DEFINE_SHUFFLE_BYTES(0, 3, 2, 1)
DEFINE_SHUFFLE_BYTES(1, 2, 3, 0)
DEFINE_SHUFFLE_BYTES(3, 0, 1, 2)
DEFINE_SHUFFLE_BYTES(3, 2, 1, 0)
