#ifndef KITCONFIG_H
#define KITCONFIG_H

/**
 * @brief Public API configurations
 * 
 * @file kitconfig.h
 * @author Tuomas Virtanen
 * @date 2018-06-25
 */

#if defined _WIN32 || defined __CYGWIN__
    #define KIT_DLL_IMPORT __declspec(dllimport)
    #define KIT_DLL_EXPORT __declspec(dllexport)
    #define KIT_DLL_LOCAL
#else
    #define KIT_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define KIT_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define KIT_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

#ifdef KIT_DLL
    #ifdef KIT_DLL_EXPORTS 
        #define KIT_API KIT_DLL_EXPORT
    #else
        #define KIT_API KIT_DLL_IMPORT
    #endif
    #define KIT_LOCAL KIT_DLL_LOCAL
#else
    #define KIT_API
    #define KIT_LOCAL
#endif

#endif // KITCONFIG_H
