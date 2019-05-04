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

#include "options/m_option.h"

struct mp_image;
struct mp_log;

struct image_writer_opts {
    int format;
    int high_bit_depth;
    int png_compression;
    int png_filter;
    int jpeg_quality;
    int jpeg_optimize;
    int jpeg_smooth;
    int jpeg_dpi;
    int jpeg_progressive;
    int jpeg_baseline;
    int jpeg_source_chroma;
    int tag_csp;
};

extern const struct image_writer_opts image_writer_opts_defaults;

extern const struct m_option image_writer_opts[];

// Return the file extension that will be used, e.g. "png".
const char *image_writer_file_ext(const struct image_writer_opts *opts);

// Return whether the selected format likely supports >8 bit per component.
bool image_writer_high_depth(const struct image_writer_opts *opts);

// Map file extension to format ID - return 0 (which is invalid) if unknown.
int image_writer_format_from_ext(const char *ext);

/*
 * Save the given image under the given filename. The parameters csp and opts
 * are optional. All pixel formats supported by swscale are supported.
 *
 * File format and compression settings are controlled via the opts parameter.
 *
 * NOTE: The fields w/h/width/height of the passed mp_image must be all set
 *       accordingly. Setting w and width or h and height to different values
 *       can be used to store snapshots of anamorphic video.
 */
bool write_image(struct mp_image *image, const struct image_writer_opts *opts,
                 const char *filename, struct mp_log *log);

/* Return the image converted to the given format. If the pixel aspect ratio is
 * not 1:1, the image is scaled as well. Returns NULL on failure.
 */
struct mp_image *convert_image(struct mp_image *image, int destfmt,
                               struct mp_log *log);

// Debugging helper.
void dump_png(struct mp_image *image, const char *filename, struct mp_log *log);
