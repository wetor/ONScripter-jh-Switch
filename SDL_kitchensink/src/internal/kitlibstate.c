#include <stdlib.h>
#include "kitchensink/internal/kitlibstate.h"

#ifdef __PPLAY__
static Kit_LibraryState _librarystate = {0, 1, 0, 3, 64, 64, NULL, NULL, ""};
#else
static Kit_LibraryState _librarystate = {0, 1, 0, 3, 64, 64, NULL, NULL};
#endif

Kit_LibraryState* Kit_GetLibraryState() {
    return &_librarystate;
}
