/* -*- C++ -*-
 *
 *  onscripter_main.cpp -- main function of ONScripter
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2019 jh10001 <jh10001@live.cn>
 *            (C) 2022-2023 yurisizuku <https://github.com/YuriSizuku>
 *            (C) 2019-2025 ONScripter-jh-Switch contributors
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ONScripter.h"
#include "Utils.h"
#include "Common.h"
#include "gbk2utf16.h"
#include "sjis2utf16.h"
#include "version.h"

#include <cstring>
#include <cstdlib>
#include <memory>

// Global ONScripter instance
ONScripter ons;

// Character encoding converter (GBK, SJIS, or UTF-8 to UTF-16)
Coding2UTF16 *coding2utf16 = nullptr;

namespace {

/**
 * Display command-line help information
 */
void showHelp()
{
    printf("ONScripter-jh for Nintendo Switch\n");
    printf("Version: %s (JH: %s, ONS: %s)\n\n", ONS_NX_VERSION, ONS_JH_VERSION, ONS_VERSION);
    printf("Usage: onscripter [option ...]\n\n");

    printf(" Load options:\n");
    printf("  -f, --font <file>              set a TTF font file\n");
    printf("  -r, --root <path>              set the root path to the archives\n");
    printf("      --save-dir <path>          set save directory\n");
    printf("      --debug:1                  print debug information\n");
    printf("      --enc:sjis                 use SJIS encoding for script\n");
    printf("      --enc:gbk                  use GBK encoding for script (default)\n");
    printf("      --enc:utf8                 use UTF-8 encoding for script\n\n");

    printf(" Render options:\n");
    printf("      --fullscreen               start in fullscreen mode\n");
    printf("      --fullscreen2              start in fullscreen mode with stretch\n");
    printf("      --window                   start in windowed mode\n");
    printf("      --width <pixels>           force window width\n");
    printf("      --height <pixels>          force window height\n");
    printf("      --sharpness <value>        use GLES to sharpen image (e.g. 3.1)\n");
    printf("      --no-video                 do not decode video\n");
    printf("      --no-vsync                 disable vertical sync\n\n");

    printf(" Other options:\n");
    printf("      --cdaudio                  use CD audio if available\n");
    printf("      --cdnumber <no>            choose the CD-ROM drive number\n");
    printf("      --registry <file>          set a registry file\n");
    printf("      --dll <file>               set a dll file\n");
    printf("      --force-button-shortcut    ignore useescspc and getenter command\n");
    printf("      --enable-wheeldown-advance advance the text on mouse wheel down\n");
    printf("      --disable-rescale          do not rescale the images in the archives\n");
    printf("      --render-font-outline      render outline instead of shadow\n");
    printf("      --edit                     enable volume/variable editing with 'z'\n");
    printf("      --key-exe <file>           set a file (*.EXE) with key table\n");
    printf("      --fontcache                cache default font\n");
    printf("      --compatible               compatibility mode\n");
    printf("  -h, --help                     show this help and exit\n");
    printf("  -v, --version                  show version information and exit\n");
    exit(0);
}

/**
 * Display version information
 */
void showVersion()
{
    printf("ONScripter-jh for Nintendo Switch\n");
    printf("Version: %s\n", ONS_NX_VERSION);
    printf("ONScripter-jh Version: %s\n", ONS_JH_VERSION);
    printf("ONScripter Version: %s\n", ONS_VERSION);
    printf("NSC Version: %d.%02d\n\n", NSC_VERSION / 100, NSC_VERSION % 100);
    printf("Written by Ogapee <ogapee@aqua.dti2.ne.jp>\n");
    printf("Modified by jh10001 <jh10001@live.cn>\n");
    printf("Enhanced by yurisizuku <https://github.com/YuriSizuku>\n");
    printf("Switch port by wetor <makisehoshimi@163.com>\n\n");
    printf("Copyright (c) 2001-2018 Ogapee.\n");
    printf("          (C) 2014-2019 jh10001\n");
    printf("          (C) 2022-2023 yurisizuku\n");
    printf("          (C) 2019-2025 ONScripter-jh-Switch contributors\n\n");
    printf("This is free software; see the source for copying conditions.\n");
    printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS\n");
    printf("FOR A PARTICULAR PURPOSE.\n");
    exit(0);
}

/**
 * Check if string matches option (handles both short and long forms)
 */
inline bool matchOption(const char* arg, const char* shortOpt, const char* longOpt)
{
    if (shortOpt && strcmp(arg + 1, shortOpt) == 0) return true;
    if (longOpt && strcmp(arg + 1, longOpt) == 0) return true;
    return false;
}

/**
 * Parse command-line options
 */
void parseOptions(int argc, char* argv[])
{
    while (argc > 0) {
        if (argv[0][0] != '-') {
            // Non-option argument
            showHelp();
        }

        const char* opt = argv[0] + 1;

        // Help and version
        if (matchOption(argv[0], "h", "-help")) {
            showHelp();
        }
        else if (matchOption(argv[0], "v", "-version")) {
            showVersion();
        }

        // Load options
        else if (matchOption(argv[0], "f", "-font")) {
            if (argc < 2) {
                utils::printError("Option --font requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setFontFile(argv[0]);
        }
        else if (matchOption(argv[0], "r", "-root")) {
            if (argc < 2) {
                utils::printError("Option --root requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setArchivePath(argv[0]);
        }
        else if (strcmp(opt, "-save-dir") == 0) {
            if (argc < 2) {
                utils::printError("Option --save-dir requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setSaveDir(argv[0]);
        }
        else if (strcmp(opt, "-debug:1") == 0) {
            ons.setDebugLevel(1);
            utils::setLogLevel(utils::LogLevel::DEBUG);
        }
        // Encoding options (from OnscripterYuri)
        else if (strcmp(opt, "-enc:sjis") == 0) {
            if (coding2utf16 == nullptr) {
                coding2utf16 = new SJIS2UTF16();
                utils::printInfo("Using SJIS encoding\n");
            }
        }
        else if (strcmp(opt, "-enc:gbk") == 0) {
            if (coding2utf16 == nullptr) {
                coding2utf16 = new GBK2UTF16();
                utils::printInfo("Using GBK encoding\n");
            }
        }
        else if (strcmp(opt, "-enc:utf8") == 0) {
            // UTF-8 mode: use GBK2UTF16 as base but enable force_utf8 flag
            if (coding2utf16 == nullptr) {
                coding2utf16 = new GBK2UTF16();
            }
            coding2utf16->force_utf8 = true;
            utils::printInfo("Using UTF-8 encoding\n");
        }

        // Render options
        else if (strcmp(opt, "-fullscreen") == 0) {
            ons.setFullscreenMode(1);
        }
        else if (strcmp(opt, "-fullscreen2") == 0) {
            ons.setFullscreenMode(2);
        }
        else if (strcmp(opt, "-window") == 0) {
            ons.setWindowMode();
        }
        else if (strcmp(opt, "-width") == 0) {
            if (argc < 2) {
                utils::printError("Option --width requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setWindowWidth(atoi(argv[0]));
        }
        else if (strcmp(opt, "-height") == 0) {
            if (argc < 2) {
                utils::printError("Option --height requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setWindowHeight(atoi(argv[0]));
        }
        else if (strcmp(opt, "-sharpness") == 0) {
            if (argc < 2) {
                utils::printError("Option --sharpness requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setSharpness(atof(argv[0]));
        }
        else if (strcmp(opt, "-no-video") == 0) {
            ons.setVideoOff();
        }
        else if (strcmp(opt, "-no-vsync") == 0) {
            ons.setVsyncOff();
        }

        // Other options
        else if (strcmp(opt, "-cdaudio") == 0) {
            ons.enableCDAudio();
        }
        else if (strcmp(opt, "-cdnumber") == 0) {
            if (argc < 2) {
                utils::printError("Option --cdnumber requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setCDNumber(atoi(argv[0]));
        }
        else if (strcmp(opt, "-registry") == 0) {
            if (argc < 2) {
                utils::printError("Option --registry requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setRegistryFile(argv[0]);
        }
        else if (strcmp(opt, "-dll") == 0) {
            if (argc < 2) {
                utils::printError("Option --dll requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setDLLFile(argv[0]);
        }
        else if (strcmp(opt, "-force-button-shortcut") == 0) {
            ons.enableButtonShortCut();
        }
        else if (strcmp(opt, "-enable-wheeldown-advance") == 0) {
            ons.enableWheelDownAdvance();
        }
        else if (strcmp(opt, "-disable-rescale") == 0) {
            ons.disableRescale();
        }
        else if (strcmp(opt, "-render-font-outline") == 0) {
            ons.renderFontOutline();
        }
        else if (strcmp(opt, "-edit") == 0) {
            ons.enableEdit();
        }
        else if (strcmp(opt, "-key-exe") == 0) {
            if (argc < 2) {
                utils::printError("Option --key-exe requires an argument\n");
                exit(1);
            }
            argc--;
            argv++;
            ons.setKeyEXE(argv[0]);
        }
        else if (strcmp(opt, "-fontcache") == 0) {
            ons.setFontCache();
        }
        else if (strcmp(opt, "-compatible") == 0) {
            ons.setCompatibilityMode();
        }
        else {
            utils::printWarning("Unknown option: %s\n", argv[0]);
        }

        argc--;
        argv++;
    }
}

/**
 * Load options from ons_args file if present
 */
void loadArgsFile()
{
    constexpr const char* ARGS_FILENAME = "ons_args";
    constexpr int MAX_ARGS = 16;
    constexpr int MAX_ARG_LENGTH = 64;

    FILE* fp = nullptr;

    // Try to open args file from archive path or current directory
    if (ons.getArchivePath()) {
        size_t pathLen = strlen(ons.getArchivePath()) + strlen(ARGS_FILENAME) + 1;
        auto fullPath = std::make_unique<char[]>(pathLen);
        snprintf(fullPath.get(), pathLen, "%s%s", ons.getArchivePath(), ARGS_FILENAME);
        fp = fopen(fullPath.get(), "r");
    }

    if (!fp) {
        fp = fopen(ARGS_FILENAME, "r");
    }

    if (!fp) {
        return; // No args file found, that's OK
    }

    utils::printDebug("Loading options from %s\n", ARGS_FILENAME);

    // Allocate argument array
    char** args = new char*[MAX_ARGS];
    int argCount = 0;

    // Pre-allocate first argument buffer
    args[argCount] = new char[MAX_ARG_LENGTH];

    // Read arguments from file
    while (argCount < MAX_ARGS && fscanf(fp, "%63s", args[argCount]) > 0) {
        argCount++;
        if (argCount < MAX_ARGS) {
            args[argCount] = new char[MAX_ARG_LENGTH];
        }
    }

    fclose(fp);

    // Parse loaded options
    if (argCount > 0) {
        parseOptions(argCount, args);
    }

    // Clean up
    for (int i = 0; i < argCount; ++i) {
        delete[] args[i];
    }
    // Clean up the extra pre-allocated buffer if not used
    if (argCount < MAX_ARGS) {
        delete[] args[argCount];
    }
    delete[] args;
}

} // anonymous namespace

#ifdef ANDROID
extern "C" {
#include <jni.h>
#include <android/log.h>

static JavaVM* jniVM = nullptr;
static jobject JavaONScripter = nullptr;
static jmethodID JavaPlayVideo = nullptr;
static jmethodID JavaGetFD = nullptr;
static jmethodID JavaMkdir = nullptr;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    (void)reserved;
    jniVM = vm;
    return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
    (void)reserved;
    jniVM = vm;
}

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores
#endif

#define JAVA_EXPORT_NAME2(name, package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name, package) JAVA_EXPORT_NAME2(name, package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name, SDL_JAVA_PACKAGE_PATH)

JNIEXPORT jint JNICALL JAVA_EXPORT_NAME(ONScripter_nativeInitJavaCallbacks)(JNIEnv* jniEnv, jobject thiz)
{
    JavaONScripter = jniEnv->NewGlobalRef(thiz);
    jclass JavaONScripterClass = jniEnv->GetObjectClass(JavaONScripter);
    JavaPlayVideo = jniEnv->GetMethodID(JavaONScripterClass, "playVideo", "([C)V");
    JavaGetFD = jniEnv->GetMethodID(JavaONScripterClass, "getFD", "([CI)I");
    JavaMkdir = jniEnv->GetMethodID(JavaONScripterClass, "mkdir", "([C)I");
    return 0;
}

JNIEXPORT jint JNICALL JAVA_EXPORT_NAME(ONScripter_nativeGetWidth)(JNIEnv* env, jobject thiz)
{
    (void)env;
    (void)thiz;
    return ons.getWidth();
}

JNIEXPORT jint JNICALL JAVA_EXPORT_NAME(ONScripter_nativeGetHeight)(JNIEnv* env, jobject thiz)
{
    (void)env;
    (void)thiz;
    return ons.getHeight();
}

void playVideoAndroid(const char* filename)
{
    JNIEnv* jniEnv = nullptr;
    jniVM->AttachCurrentThread(&jniEnv, nullptr);

    if (!jniEnv) {
        __android_log_print(ANDROID_LOG_ERROR, "ONS",
            "ONScripter::playVideoAndroid: Java VM AttachCurrentThread() failed");
        return;
    }

    size_t len = strlen(filename);
    auto jc = std::make_unique<jchar[]>(len);
    for (size_t i = 0; i < len; i++) {
        jc[i] = filename[i];
    }

    jcharArray jca = jniEnv->NewCharArray(len);
    jniEnv->SetCharArrayRegion(jca, 0, len, jc.get());
    jniEnv->CallVoidMethod(JavaONScripter, JavaPlayVideo, jca);
    jniEnv->DeleteLocalRef(jca);
}

#undef fopen
FILE* fopen_ons(const char* path, const char* mode)
{
    int mode2 = (mode[0] == 'w') ? 1 : 0;

    FILE* fp = fopen(path, mode);
    if (fp || mode2 == 0) {
        return fp;
    }

    JNIEnv* jniEnv = nullptr;
    jniVM->AttachCurrentThread(&jniEnv, nullptr);

    if (!jniEnv) {
        __android_log_print(ANDROID_LOG_ERROR, "ONS",
            "ONScripter::getFD: Java VM AttachCurrentThread() failed");
        return nullptr;
    }

    size_t len = strlen(path);
    auto jc = std::make_unique<jchar[]>(len);
    for (size_t i = 0; i < len; i++) {
        jc[i] = path[i];
    }

    jcharArray jca = jniEnv->NewCharArray(len);
    jniEnv->SetCharArrayRegion(jca, 0, len, jc.get());
    int fd = jniEnv->CallIntMethod(JavaONScripter, JavaGetFD, jca, mode2);
    jniEnv->DeleteLocalRef(jca);

    return fdopen(fd, mode);
}

#undef mkdir
int mkdir_ons(const char* pathname, mode_t mode)
{
    (void)mode;

    JNIEnv* jniEnv = nullptr;
    jniVM->AttachCurrentThread(&jniEnv, nullptr);

    if (!jniEnv) {
        __android_log_print(ANDROID_LOG_ERROR, "ONS",
            "ONScripter::mkdir: Java VM AttachCurrentThread() failed");
        return -1;
    }

    size_t len = strlen(pathname);
    auto jc = std::make_unique<jchar[]>(len);
    for (size_t i = 0; i < len; i++) {
        jc[i] = pathname[i];
    }

    jcharArray jca = jniEnv->NewCharArray(len);
    jniEnv->SetCharArrayRegion(jca, 0, len, jc.get());
    int ret = jniEnv->CallIntMethod(JavaONScripter, JavaMkdir, jca);
    jniEnv->DeleteLocalRef(jca);

    return ret;
}

} // extern "C"
#endif // ANDROID

/**
 * ONScripter main entry point
 * Called by platform-specific main() after system initialization
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return 0 on success, negative value on error
 */
int OnsMain(int argc, char* argv[])
{
    utils::printInfo("ONScripter-jh for Nintendo Switch\n");
    utils::printInfo("Version: %s (JH: %s, ONS: %s, NSC: %d.%02d)\n",
        ONS_NX_VERSION, ONS_JH_VERSION, ONS_VERSION,
        NSC_VERSION / 100, NSC_VERSION % 100);
    utils::printInfo("Platform: %s\n\n", ONS_PLATFORM_NAME);

#if defined(SWITCH)
    // Nintendo Switch specific initialization
    ons.setCompatibilityMode();
    ons.disableRescale();
    ons.enableButtonShortCut();
    utils::printDebug("Switch-specific options enabled\n");
#endif

    // Parse command-line options (skip program name)
    if (argc > 1) {
        parseOptions(argc - 1, argv + 1);
    }

    // Load additional options from args file
    loadArgsFile();

    // Set default encoding if not specified
    if (coding2utf16 == nullptr) {
        coding2utf16 = new GBK2UTF16();
        utils::printDebug("Using default GBK encoding\n");
    }

    // Initialize and run ONScripter
    utils::printInfo("Opening script...\n");
    if (ons.openScript()) {
        utils::printError("Failed to open script\n");
        return -1;
    }

    utils::printInfo("Initializing ONScripter...\n");
    if (ons.init()) {
        utils::printError("Failed to initialize ONScripter\n");
        return -1;
    }

    utils::printInfo("Starting execution...\n");
    ons.executeLabel();

    utils::printInfo("ONScripter exited normally\n");
    return 0;
}
