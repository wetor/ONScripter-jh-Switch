#include <stdlib.h>
#include <assert.h>

#include "kitchensink/kiterror.h"
#include "kitchensink/internal/subtitle/kitsubtitlepacket.h"
#include "kitchensink/internal/subtitle/renderers/kitsubrenderer.h"


Kit_SubtitleRenderer* Kit_CreateSubtitleRenderer(Kit_Decoder *dec) {
    // Allocate renderer and make sure allocation was a success
    Kit_SubtitleRenderer *ren = calloc(1, sizeof(Kit_SubtitleRenderer));
    if(ren == NULL) {
        Kit_SetError("Unable to allocate kit subtitle renderer");
        return NULL;
    }
    ren->dec = dec;
    return ren;
}

void Kit_RunSubtitleRenderer(Kit_SubtitleRenderer *ren, void *src, double pts, double start, double end) {
    if(ren == NULL)
        return;
    if(ren->ren_render != NULL)
        ren->ren_render(ren, src, pts, start, end);
}

int Kit_GetSubtitleRendererData(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, SDL_Texture *texture, double current_pts) {
    if(ren == NULL)
        return 0;
    if(ren->ren_get_data != NULL)
        return ren->ren_get_data(ren, atlas, texture, current_pts);
    return 0;
}

int Kit_GetSubtitleRendererDataRaw(Kit_SubtitleRenderer *ren, Kit_TextureAtlas *atlas, void *data, double current_pts) {
    if(ren == NULL)
        return 0;
    if(ren->ren_get_data_raw != NULL)
        return ren->ren_get_data_raw(ren, atlas, data, current_pts);
    return 0;
}

void Kit_SetSubtitleRendererSize(Kit_SubtitleRenderer *ren, int w, int h) {
    if(ren == NULL)
        return;
    if(ren->ren_set_size != NULL)
        ren->ren_set_size(ren, w, h);
}

void Kit_CloseSubtitleRenderer(Kit_SubtitleRenderer *ren) {
    if(ren == NULL)
        return;
    if(ren->ren_close != NULL)
        ren->ren_close(ren);
    free(ren);
}
