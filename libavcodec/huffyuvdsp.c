/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>

#include "config.h"
#include "libavutil/attributes.h"
#include "mathops.h"
#include "huffyuvdsp.h"

static void add_int16_c(uint16_t *dst, const uint16_t *src, unsigned mask, int w){
    long i;
    unsigned long pw_lsb = (mask >> 1) * 0x0001000100010001ULL;
    unsigned long pw_msb = pw_lsb +  0x0001000100010001ULL;
    for (i = 0; i <= w - (int)sizeof(long)/2; i += sizeof(long)/2) {
        long a = *(long*)(src+i);
        long b = *(long*)(dst+i);
        *(long*)(dst+i) = ((a&pw_lsb) + (b&pw_lsb)) ^ ((a^b)&pw_msb);
    }
    for(; i<w; i++)
        dst[i] = (dst[i] + src[i]) & mask;
}

static void add_hfyu_median_pred_int16_c(uint16_t *dst, const uint16_t *src, const uint16_t *diff, unsigned mask, int w, int *left, int *left_top){
    int i;
    uint16_t l, lt;

    l  = *left;
    lt = *left_top;

    for(i=0; i<w; i++){
        l  = (mid_pred(l, src[i], (l + src[i] - lt) & mask) + diff[i]) & mask;
        lt = src[i];
        dst[i] = l;
    }

    *left     = l;
    *left_top = lt;
}

static void add_hfyu_left_pred_bgr32_c(uint8_t *dst, const uint8_t *src,
                                       intptr_t w, uint8_t *left)
{
    int i;
    uint8_t r = left[R], g = left[G], b = left[B], a = left[A];

    for (i = 0; i < w; i++) {
        b += src[4 * i + B];
        g += src[4 * i + G];
        r += src[4 * i + R];
        a += src[4 * i + A];

        dst[4 * i + B] = b;
        dst[4 * i + G] = g;
        dst[4 * i + R] = r;
        dst[4 * i + A] = a;
    }

    left[B] = b;
    left[G] = g;
    left[R] = r;
    left[A] = a;
}

av_cold void ff_huffyuvdsp_init(HuffYUVDSPContext *c, AVCodecContext *avctx)
{
    c->add_int16 = add_int16_c;
    c->add_hfyu_median_pred_int16 = add_hfyu_median_pred_int16_c;
    c->add_hfyu_left_pred_bgr32 = add_hfyu_left_pred_bgr32_c;

    if (ARCH_X86)
        ff_huffyuvdsp_init_x86(c, avctx);
}
