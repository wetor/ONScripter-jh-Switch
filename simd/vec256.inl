/* -*- C++ -*-
*
*  vec256.inl
*
*  Copyright (C) 2018 jh10001 <jh10001@live.cn>
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

#ifndef __SIMD_H__
#error "This file must be included through simd.h"
#endif

namespace simd {
  //Load
  inline ivec256 load256_a(const void *m) {
#if USE_SIMD_X86_AVX2
    return _mm256_load_si256(reinterpret_cast<const __m256i*>(m));
#endif
  }

  inline ivec256 load256_u(const void *m) {
#if USE_SIMD_X86_AVX2
    return _mm256_lddqu_si256(reinterpret_cast<const __m256i*>(m));
#endif
  }

  //Store
  inline void store256_a(void* m, __m256i a) {
#ifdef USE_SIMD_X86_AVX2
    _mm256_store_si256(reinterpret_cast<__m256i*>(m), a);
#endif
  }

  inline void store256_u(void* m, __m256i a) {
#ifdef USE_SIMD_X86_AVX2
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(m), a);
#endif
  }
}
