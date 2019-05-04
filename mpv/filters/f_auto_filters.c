#include <math.h>

#include "common/common.h"
#include "common/msg.h"
#include "options/m_config.h"
#include "options/options.h"
#include "video/mp_image.h"

#include "f_auto_filters.h"
#include "f_swscale.h"
#include "f_utils.h"
#include "filter.h"
#include "filter_internal.h"
#include "user_filters.h"

struct deint_priv {
    struct mp_subfilter sub;
    int prev_imgfmt;
    int prev_setting;
    struct m_config_cache *opts;
};

static void deint_process(struct mp_filter *f)
{
    struct deint_priv *p = f->priv;

    if (!mp_subfilter_read(&p->sub))
        return;

    struct mp_frame frame = p->sub.frame;

    if (mp_frame_is_signaling(frame)) {
        mp_subfilter_continue(&p->sub);
        return;
    }

    if (frame.type != MP_FRAME_VIDEO) {
        MP_ERR(f, "video input required!\n");
        mp_filter_internal_mark_failed(f);
        return;
    }

    m_config_cache_update(p->opts);
    struct filter_opts *opts = p->opts->opts;

    if (!opts->deinterlace)
        mp_subfilter_destroy(&p->sub);

    struct mp_image *img = frame.data;

    if (img->imgfmt == p->prev_imgfmt && p->prev_setting == opts->deinterlace) {
        mp_subfilter_continue(&p->sub);
        return;
    }

    if (!mp_subfilter_drain_destroy(&p->sub))
        return;

    assert(!p->sub.filter);

    p->prev_imgfmt = img->imgfmt;
    p->prev_setting = opts->deinterlace;
    if (!p->prev_setting) {
        mp_subfilter_continue(&p->sub);
        return;
    }

    if (img->imgfmt == IMGFMT_VDPAU) {
        char *args[] = {"deint", "yes", NULL};
        p->sub.filter =
            mp_create_user_filter(f, MP_OUTPUT_CHAIN_VIDEO, "vdpaupp", args);
    } else if (img->imgfmt == IMGFMT_VAAPI) {
        p->sub.filter =
            mp_create_user_filter(f, MP_OUTPUT_CHAIN_VIDEO, "vavpp", NULL);
    } else if (img->imgfmt == IMGFMT_D3D11) {
        p->sub.filter =
            mp_create_user_filter(f, MP_OUTPUT_CHAIN_VIDEO, "d3d11vpp", NULL);
    } else if (img->imgfmt == IMGFMT_CUDA) {
        char *args[] = {"mode", "send_field", NULL};
        p->sub.filter =
            mp_create_user_filter(f, MP_OUTPUT_CHAIN_VIDEO, "yadif_cuda", args);
    } else if (mp_sws_supports_input(img->imgfmt)) {
        char *args[] = {"mode", "send_field", NULL};
        p->sub.filter =
            mp_create_user_filter(f, MP_OUTPUT_CHAIN_VIDEO, "yadif", args);
    } else {
        MP_ERR(f, "no deinterlace filter available for this format\n");
        mp_subfilter_continue(&p->sub);
        return;
    }

    if (!p->sub.filter)
        MP_ERR(f, "creating deinterlacer failed\n");

    mp_subfilter_continue(&p->sub);
}

static void deint_reset(struct mp_filter *f)
{
    struct deint_priv *p = f->priv;

    mp_subfilter_reset(&p->sub);
}

static void deint_destroy(struct mp_filter *f)
{
    struct deint_priv *p = f->priv;

    mp_subfilter_reset(&p->sub);
    TA_FREEP(&p->sub.filter);
}

static bool deint_command(struct mp_filter *f, struct mp_filter_command *cmd)
{
    struct deint_priv *p = f->priv;

    if (cmd->type == MP_FILTER_COMMAND_IS_ACTIVE) {
        cmd->is_active = !!p->sub.filter;
        return true;
    }
    return false;
}

static const struct mp_filter_info deint_filter = {
    .name = "deint",
    .priv_size = sizeof(struct deint_priv),
    .command = deint_command,
    .process = deint_process,
    .reset = deint_reset,
    .destroy = deint_destroy,
};

struct mp_filter *mp_deint_create(struct mp_filter *parent)
{
    struct mp_filter *f = mp_filter_create(parent, &deint_filter);
    if (!f)
        return NULL;

    struct deint_priv *p = f->priv;

    p->sub.in = mp_filter_add_pin(f, MP_PIN_IN, "in");
    p->sub.out = mp_filter_add_pin(f, MP_PIN_OUT, "out");

    p->opts = m_config_cache_alloc(f, f->global, &filter_conf);

    return f;
}

struct rotate_priv {
    struct mp_subfilter sub;
    int prev_rotate;
    int prev_imgfmt;
    int target_rotate;
};

static void rotate_process(struct mp_filter *f)
{
    struct rotate_priv *p = f->priv;

    if (!mp_subfilter_read(&p->sub))
        return;

    struct mp_frame frame = p->sub.frame;

    if (mp_frame_is_signaling(frame)) {
        mp_subfilter_continue(&p->sub);
        return;
    }

    if (frame.type != MP_FRAME_VIDEO) {
        MP_ERR(f, "video input required!\n");
        return;
    }

    struct mp_image *img = frame.data;

    if (img->params.rotate == p->prev_rotate &&
        img->imgfmt == p->prev_imgfmt)
    {
        img->params.rotate = p->target_rotate;
        mp_subfilter_continue(&p->sub);
        return;
    }

    if (!mp_subfilter_drain_destroy(&p->sub))
        return;

    assert(!p->sub.filter);

    int rotate = p->prev_rotate = img->params.rotate;
    p->target_rotate = rotate;
    p->prev_imgfmt = img->imgfmt;

    struct mp_stream_info *info = mp_filter_find_stream_info(f);
    if (rotate == 0 || (info && info->rotate90 && !(rotate % 90))) {
        mp_subfilter_continue(&p->sub);
        return;
    }

    if (!mp_sws_supports_input(img->imgfmt)) {
        MP_ERR(f, "Video rotation with this format not supported\n");
        mp_subfilter_continue(&p->sub);
        return;
    }

    double angle = rotate / 360.0 * M_PI * 2;
    char *args[] = {"angle", mp_tprintf(30, "%f", angle),
                    "ow", mp_tprintf(30, "rotw(%f)", angle),
                    "oh", mp_tprintf(30, "roth(%f)", angle),
                    NULL};
    p->sub.filter =
        mp_create_user_filter(f, MP_OUTPUT_CHAIN_VIDEO, "rotate", args);

    if (p->sub.filter) {
        MP_INFO(f, "Inserting rotation filter.\n");
        p->target_rotate = 0;
    } else {
        MP_ERR(f, "could not create rotation filter\n");
    }

    mp_subfilter_continue(&p->sub);
}

static void rotate_reset(struct mp_filter *f)
{
    struct rotate_priv *p = f->priv;

    mp_subfilter_reset(&p->sub);
}

static void rotate_destroy(struct mp_filter *f)
{
    struct rotate_priv *p = f->priv;

    mp_subfilter_reset(&p->sub);
    TA_FREEP(&p->sub.filter);
}

static bool rotate_command(struct mp_filter *f, struct mp_filter_command *cmd)
{
    struct rotate_priv *p = f->priv;

    if (cmd->type == MP_FILTER_COMMAND_IS_ACTIVE) {
        cmd->is_active = !!p->sub.filter;
        return true;
    }
    return false;
}

static const struct mp_filter_info rotate_filter = {
    .name = "autorotate",
    .priv_size = sizeof(struct rotate_priv),
    .command = rotate_command,
    .process = rotate_process,
    .reset = rotate_reset,
    .destroy = rotate_destroy,
};

struct mp_filter *mp_autorotate_create(struct mp_filter *parent)
{
    struct mp_filter *f = mp_filter_create(parent, &rotate_filter);
    if (!f)
        return NULL;

    struct rotate_priv *p = f->priv;
    p->prev_rotate = -1;

    p->sub.in = mp_filter_add_pin(f, MP_PIN_IN, "in");
    p->sub.out = mp_filter_add_pin(f, MP_PIN_OUT, "out");

    return f;
}

struct aspeed_priv {
    struct mp_subfilter sub;
    double cur_speed;
};

static void aspeed_process(struct mp_filter *f)
{
    struct aspeed_priv *p = f->priv;

    if (!mp_subfilter_read(&p->sub))
        return;

    if (fabs(p->cur_speed - 1.0) < 1e-8) {
        if (p->sub.filter)
            MP_VERBOSE(f, "removing scaletempo\n");
        if (!mp_subfilter_drain_destroy(&p->sub))
            return;
    } else if (!p->sub.filter) {
        MP_VERBOSE(f, "adding scaletempo\n");
        p->sub.filter =
            mp_create_user_filter(f, MP_OUTPUT_CHAIN_AUDIO, "scaletempo", NULL);
        if (!p->sub.filter) {
            MP_ERR(f, "could not create scaletempo filter\n");
            mp_subfilter_continue(&p->sub);
            return;
        }
    }

    if (p->sub.filter) {
        struct mp_filter_command cmd = {
            .type = MP_FILTER_COMMAND_SET_SPEED,
            .speed = p->cur_speed,
        };
        mp_filter_command(p->sub.filter, &cmd);
    }

    mp_subfilter_continue(&p->sub);
}

static bool aspeed_command(struct mp_filter *f, struct mp_filter_command *cmd)
{
    struct aspeed_priv *p = f->priv;

    if (cmd->type == MP_FILTER_COMMAND_SET_SPEED) {
        p->cur_speed = cmd->speed;
        return true;
    }

    if (cmd->type == MP_FILTER_COMMAND_IS_ACTIVE) {
        cmd->is_active = !!p->sub.filter;
        return true;
    }

    return false;
}

static void aspeed_reset(struct mp_filter *f)
{
    struct aspeed_priv *p = f->priv;

    mp_subfilter_reset(&p->sub);
}

static void aspeed_destroy(struct mp_filter *f)
{
    struct aspeed_priv *p = f->priv;

    mp_subfilter_reset(&p->sub);
    TA_FREEP(&p->sub.filter);
}

static const struct mp_filter_info aspeed_filter = {
    .name = "autoaspeed",
    .priv_size = sizeof(struct aspeed_priv),
    .command = aspeed_command,
    .process = aspeed_process,
    .reset = aspeed_reset,
    .destroy = aspeed_destroy,
};

struct mp_filter *mp_autoaspeed_create(struct mp_filter *parent)
{
    struct mp_filter *f = mp_filter_create(parent, &aspeed_filter);
    if (!f)
        return NULL;

    struct aspeed_priv *p = f->priv;
    p->cur_speed = 1.0;

    p->sub.in = mp_filter_add_pin(f, MP_PIN_IN, "in");
    p->sub.out = mp_filter_add_pin(f, MP_PIN_OUT, "out");

    return f;
}
