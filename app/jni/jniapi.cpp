//
// Copyright 2011 Tero Saarni
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <stdint.h>
#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer

#include "jniapi.h"
#include "renderer.h"

#define LOG_TAG "uhmitest"
#include <pr.h>

static ANativeWindow *window = 0;
static Renderer *renderer = 0;

#ifndef __unused
#define __unused __attribute__((unused))
#endif

JNIEXPORT void JNICALL Java_tsaarni_nativeeglexample_NativeEglExample_nativeOnStart(__unused JNIEnv* jenv, __unused jobject obj)
{
    pr_info("nativeOnStart");
    renderer = new Renderer();
}

JNIEXPORT void JNICALL Java_tsaarni_nativeeglexample_NativeEglExample_nativeOnResume(__unused JNIEnv* jenv, __unused jobject obj)
{
    pr_info("nativeOnResume");
    renderer->resume();
}

JNIEXPORT void JNICALL Java_tsaarni_nativeeglexample_NativeEglExample_nativeOnPause(__unused JNIEnv* jenv, __unused jobject obj)
{
    pr_info("nativeOnPause");
    renderer->pause();
}

JNIEXPORT void JNICALL Java_tsaarni_nativeeglexample_NativeEglExample_nativeOnStop(__unused JNIEnv* jenv, __unused jobject obj)
{
    pr_info("nativeOnStop");
    delete renderer;
    renderer = 0;
}

JNIEXPORT void JNICALL Java_tsaarni_nativeeglexample_NativeEglExample_nativeSetSurface(__unused JNIEnv* jenv, __unused jobject obj, jobject surface)
{
    pr_info("nativeSetSurface(%p)", surface);
    if (surface != 0) {
        window = ANativeWindow_fromSurface(jenv, surface);
        renderer->setWindow(window);
    } else {
        ANativeWindow_release(window);
        window = 0;
    }
}

JNIEXPORT void JNICALL Java_tsaarni_nativeeglexample_NativeEglExample_nativeChangeContext(__unused JNIEnv* jenv, __unused jobject obj)
{
    pr_info("nativeChangeContext");
    if (renderer)
        renderer->changeContext();
}
