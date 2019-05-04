/*
 * This file is part of mpv.
 *
 * mpv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * mpv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MP_AUDIO_BUFFER_H
#define MP_AUDIO_BUFFER_H

struct mp_audio_buffer;
struct mp_chmap;

struct mp_audio_buffer *mp_audio_buffer_create(void *talloc_ctx);
void mp_audio_buffer_reinit_fmt(struct mp_audio_buffer *ab, int format,
                                const struct mp_chmap *channels, int srate);
void mp_audio_buffer_preallocate_min(struct mp_audio_buffer *ab, int samples);
int mp_audio_buffer_get_write_available(struct mp_audio_buffer *ab);
void mp_audio_buffer_append(struct mp_audio_buffer *ab, void **ptr, int samples);
void mp_audio_buffer_prepend_silence(struct mp_audio_buffer *ab, int samples);
void mp_audio_buffer_duplicate(struct mp_audio_buffer *ab, int samples);
void mp_audio_buffer_peek(struct mp_audio_buffer *ab, uint8_t ***ptr,
                          int *samples);
void mp_audio_buffer_skip(struct mp_audio_buffer *ab, int samples);
void mp_audio_buffer_clear(struct mp_audio_buffer *ab);
int mp_audio_buffer_samples(struct mp_audio_buffer *ab);
double mp_audio_buffer_seconds(struct mp_audio_buffer *ab);

#endif
