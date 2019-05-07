# SDL_kitchensink

[![Build Status](https://travis-ci.org/katajakasa/SDL_kitchensink.svg?branch=master)](https://travis-ci.org/katajakasa/SDL_kitchensink)
[![Quality Gate](https://sonarcloud.io/api/project_badges/measure?project=sdl_kitchensink&metric=alert_status)](https://sonarcloud.io/dashboard?id=sdl_kitchensink)

FFmpeg and SDL2 based library for audio and video playback, written in C99.

Documentation is available at http://katajakasa.github.io/SDL_kitchensink/

Features:
* Decoding video, audio and subtitles via FFmpeg
* Dumping video and subtitle data on SDL_textures
* Dumping audio data in the usual mono/stereo interleaved formats
* Automatic audio and video conversion to SDL2 friendly formats
* Synchronizing video & audio to clock
* Seeking forwards and backwards
* Bitmap, text and SSA/ASS subtitle support

Note! Master branch is for the development of v1.0.0 series. v0 can be found in the 
rel-kitchensink-0 branch. v0 is no longer in active development and only bug- and security-fixes
are accepted.

## 1. Library requirements

Build requirements:
* CMake (>=3.0)
* GCC (C99 support required)

Library requirements:
* SDL2 (>=2.0.5)
* FFmpeg (>=3.0)
* libass (optional, supports runtime linking via SDL_LoadSO)

Note that Clang might work, but is not tested. Older SDL2 and FFmpeg library versions
may or may not work; versions noted here are the only ones tested.

### 1.1. Debian / Ubuntu

```
sudo apt-get install libsdl2-dev libavcodec-dev libavdevice-dev libavfilter-dev \
libavformat-dev libavresample-dev libavutil-dev libswresample-dev libswscale-dev \
libpostproc-dev libass-dev
```

### 1.2. MSYS2 64bit

These are for x86_64. For 32bit installation, just change the package names a bit .
```
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-ffmpeg mingw-w64-x86_64-libass
```

## 2. Compiling

By default, both static and dynamic libraries are built.
* Dynamic library is called libSDL_kitchensink.dll or .so
* Static library is called libSDL_kitchensink_static.a
* If you build in debug mode (```-DCMAKE_BUILD_TYPE=Debug```), libraries will be postfixed with 'd'.

Change CMAKE_INSTALL_PREFIX as necessary to change the installation path. The files will be installed to
* CMAKE_INSTALL_PREFIX/lib for libraries (.dll.a, .a, etc.)
* CMAKE_INSTALL_PREFIX/bin for binaries (.dll, .so)
* CMAKE_INSTALL_PREFIX/include for headers

### 2.1. Building the libraries on Debian/Ubuntu

1. ```mkdir build && cd build```
2. ```cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..```
3. ```make -j```
4. ```sudo make install```

### 2.2. Building the libraries on MSYS2

1. ```mkdir build && cd build```
2. ```cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..```
3. ```make```
4. ```make install```

### 2.3. Building examples

Just add ```-DBUILD_EXAMPLES=1``` to cmake arguments and rebuild.

### 2.4. Building with AddressSanitizer

This is for development/debugging use only!

Make sure llvm is installed, then add ```-DUSE_ASAN=1``` to the cmake arguments and rebuild. Note that ASAN is not
supported on all OSes (eg. windows).

After building, you can run with the following (make sure to set correct llvm-symbolizer path):
```
ASAN_OPTIONS=symbolize=1 ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer ./examplevideo <my videofile>
```

## 3. Why the name SDL_kitchensink

Because pulling major blob of library code like ffmpeg feels like bringing in a whole house with its
kitchensink and everything to the project. Also, it sounded funny. Also, SDL_ffmpeg is already reserved :(

## 4. Examples

Please see examples directory. You can also take a look at unittests for some help.
Note that examples are NOT meant for any kind of real life use; they are only meant to
show simple use cases for the library.

## 5. FFMPEG & licensing

Note that FFmpeg has a rather complex license. Please take a look at 
[FFmpeg Legal page](http://ffmpeg.org/legal.html) for details.

## 6. License

```
The MIT License (MIT)

Copyright (c) 2018 Tuomas Virtanen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
