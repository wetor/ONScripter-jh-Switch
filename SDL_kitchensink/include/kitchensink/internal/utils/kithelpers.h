#ifndef KITHELPERS_H
#define KITHELPERS_H

#include <stdbool.h>
#include <libavformat/avformat.h>
#include "kitchensink/kitconfig.h"

KIT_LOCAL double _GetSystemTime();
KIT_LOCAL bool attachment_is_font(AVStream *stream);

#endif // KITHELPERS_H
