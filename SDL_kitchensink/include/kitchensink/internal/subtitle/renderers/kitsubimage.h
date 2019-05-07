#ifndef KITSUBIMAGE_H
#define KITSUBIMAGE_H

#include "kitchensink/kitconfig.h"
#include "kitchensink/internal/kitdecoder.h"
#include "kitchensink/internal/subtitle/renderers/kitsubrenderer.h"

KIT_LOCAL Kit_SubtitleRenderer* Kit_CreateImageSubtitleRenderer(
    Kit_Decoder *dec, int video_w, int video_h, int screen_w, int screen_h);

#endif // KITSUBIMAGE_H
