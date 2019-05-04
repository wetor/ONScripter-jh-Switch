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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include <libavutil/rational.h>

#include "common/msg.h"
#include "common/common.h"
#include "filters/f_autoconvert.h"
#include "filters/filter.h"
#include "filters/filter_internal.h"
#include "filters/user_filters.h"
#include "video/img_format.h"
#include "video/mp_image.h"

#include "options/m_option.h"

struct priv {
    struct vf_format_opts *opts;
    struct mp_pin *in_pin;
};

struct vf_format_opts {
    int fmt;
    int outfmt;
    int colormatrix;
    int colorlevels;
    int primaries;
    int gamma;
    float sig_peak;
    int light;
    int chroma_location;
    int stereo_in;
    int rotate;
    int dw, dh;
    double dar;
    int spherical;
    float spherical_ref_angles[3];
};

static void vf_format_process(struct mp_filter *f)
{
    struct priv *priv = f->priv;
    struct vf_format_opts *p = priv->opts;

    if (!mp_pin_can_transfer_data(f->ppins[1], priv->in_pin))
        return;

    struct mp_frame frame = mp_pin_out_read(priv->in_pin);

    if (mp_frame_is_signaling(frame)) {
        mp_pin_in_write(f->ppins[1], frame);
        return;
    }
    if (frame.type != MP_FRAME_VIDEO) {
        MP_ERR(f, "unsupported frame type\n");
        mp_frame_unref(&frame);
        mp_filter_internal_mark_failed(f);
        return;
    }

    struct mp_image *img = frame.data;
    struct mp_image_params *out = &img->params;

    if (p->outfmt)
        out->imgfmt = p->outfmt;
    if (p->colormatrix)
        out->color.space = p->colormatrix;
    if (p->colorlevels)
        out->color.levels = p->colorlevels;
    if (p->primaries)
        out->color.primaries = p->primaries;
    if (p->gamma) {
        enum mp_csp_trc in_gamma = p->gamma;
        out->color.gamma = p->gamma;
        if (in_gamma != out->color.gamma) {
            // When changing the gamma function explicitly, also reset stuff
            // related to the gamma function since that information will almost
            // surely be false now and have to be re-inferred
            out->color.sig_peak = 0.0;
            out->color.light = MP_CSP_LIGHT_AUTO;
        }
    }
    if (p->sig_peak)
        out->color.sig_peak = p->sig_peak;
    if (p->light)
        out->color.light = p->light;
    if (p->chroma_location)
        out->chroma_location = p->chroma_location;
    if (p->stereo_in)
        out->stereo3d = p->stereo_in;
    if (p->rotate >= 0)
        out->rotate = p->rotate;

    AVRational dsize;
    mp_image_params_get_dsize(out, &dsize.num, &dsize.den);
    if (p->dw > 0)
        dsize.num = p->dw;
    if (p->dh > 0)
        dsize.den = p->dh;
    if (p->dar > 0)
        dsize = av_d2q(p->dar, INT_MAX);
    mp_image_params_set_dsize(out, dsize.num, dsize.den);

    if (p->spherical)
        out->spherical.type = p->spherical;
    for (int n = 0; n < 3; n++) {
        if (isfinite(p->spherical_ref_angles[n]))
            out->spherical.ref_angles[n] = p->spherical_ref_angles[n];
    }

    // Make sure the user-overrides are consistent (no RGB csp for YUV, etc.).
    mp_image_params_guess_csp(out);

    mp_pin_in_write(f->ppins[1], frame);
}

static const struct mp_filter_info vf_format_filter = {
    .name = "format",
    .process = vf_format_process,
    .priv_size = sizeof(struct priv),
};

static struct mp_filter *vf_format_create(struct mp_filter *parent, void *options)
{
    struct mp_filter *f = mp_filter_create(parent, &vf_format_filter);
    if (!f) {
        talloc_free(options);
        return NULL;
    }

    struct priv *priv = f->priv;
    priv->opts = talloc_steal(priv, options);

    mp_filter_add_pin(f, MP_PIN_IN, "in");
    mp_filter_add_pin(f, MP_PIN_OUT, "out");

    struct mp_autoconvert *conv = mp_autoconvert_create(f);
    if (!conv) {
        talloc_free(f);
        return NULL;
    }

    if (priv->opts->fmt)
        mp_autoconvert_add_imgfmt(conv, priv->opts->fmt, 0);

    priv->in_pin = conv->f->pins[1];
    mp_pin_connect(conv->f->pins[0], f->ppins[0]);

    return f;
}

#define OPT_BASE_STRUCT struct vf_format_opts
static const m_option_t vf_opts_fields[] = {
    OPT_IMAGEFORMAT("fmt", fmt, 0),
    OPT_CHOICE_C("colormatrix", colormatrix, 0, mp_csp_names),
    OPT_CHOICE_C("colorlevels", colorlevels, 0, mp_csp_levels_names),
    OPT_CHOICE_C("primaries", primaries, 0, mp_csp_prim_names),
    OPT_CHOICE_C("gamma", gamma, 0, mp_csp_trc_names),
    OPT_FLOAT("sig-peak", sig_peak, 0),
    OPT_CHOICE_C("light", light, 0, mp_csp_light_names),
    OPT_CHOICE_C("chroma-location", chroma_location, 0, mp_chroma_names),
    OPT_CHOICE_C("stereo-in", stereo_in, 0, mp_stereo3d_names),
    OPT_INTRANGE("rotate", rotate, 0, -1, 359),
    OPT_INT("dw", dw, 0),
    OPT_INT("dh", dh, 0),
    OPT_DOUBLE("dar", dar, 0),
    OPT_CHOICE_C("spherical", spherical, 0, mp_spherical_names),
    OPT_FLOAT("spherical-yaw", spherical_ref_angles[0], 0),
    OPT_FLOAT("spherical-pitch", spherical_ref_angles[1], 0),
    OPT_FLOAT("spherical-roll", spherical_ref_angles[2], 0),
    OPT_REMOVED("outputlevels", "use the --video-output-levels global option"),
    OPT_REMOVED("peak", "use sig-peak instead (changed value scale!)"),
    {0}
};

const struct mp_user_filter_entry vf_format = {
    .desc = {
        .description = "force output format",
        .name = "format",
        .priv_size = sizeof(OPT_BASE_STRUCT),
        .priv_defaults = &(const OPT_BASE_STRUCT){
            .rotate = -1,
            .spherical_ref_angles = {NAN, NAN, NAN},
        },
        .options = vf_opts_fields,
    },
    .create = vf_format_create,
};
