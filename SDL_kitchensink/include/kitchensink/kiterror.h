#ifndef KITERROR_H
#define KITERROR_H

/**
 * @brief Error handling functions
 * 
 * @file kiterror.h
 * @author Tuomas Virtanen
 * @date 2018-06-25
 */

#include "kitchensink/kitconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the latest error. This is set by SDL_kitchensink library functions on error.
 * 
 * @return Error message or NULL
 */
KIT_API const char* Kit_GetError();

/**
 * @brief Sets the error message. This should really only be used by the library.
 * 
 * @param fmt Message format
 * @param ... Message arguments
 */
KIT_API void Kit_SetError(const char* fmt, ...);

/**
 * @brief Clears latest error message. After this, Kit_GetError() will return NULL.
 */
KIT_API void Kit_ClearError();

#ifdef __cplusplus
}
#endif

#endif // KITERROR_H
