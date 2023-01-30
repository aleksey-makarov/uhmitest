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

#include <thread>
#include <mutex>
#include <chrono>

#include <libuhmigl.h>
#include <es2gears.h>
#define LOG_TAG "uhmitest"
#include <pr.h>

#include "renderer.h"

Renderer::Renderer()
    : _msg(0), _window(0), _thread(threadFunc, this)
{
    pr_info();
}

Renderer::~Renderer()
{
    pr_info();
    push(MSG_EXIT, 0);
    _thread.join();
}

void Renderer::push(int msg, ANativeWindow *window)
{
    int msg_orig;
    pr_info();
    {
        const std::lock_guard<std::mutex> lock(_mutex);
        msg_orig = _msg;
        _msg |= msg;
        if (msg & MSG_SET_WINDOW)
            _window = window;
        _cond.notify_all();
    }

    if (msg_orig & msg)
        pr_err("overflow: msg: %d, msg_orig: %d", msg, msg_orig);
}

int Renderer::pop(ANativeWindow **window)
{
    int msg_orig;
    const std::lock_guard<std::mutex> lock(_mutex);

    if (_msg & MSG_SET_WINDOW)
        *window = _window;
    msg_orig = _msg;
    _msg = 0;

    return msg_orig;
}

int Renderer::popWait(ANativeWindow **window)
{
    int msg_orig;
    std::unique_lock<std::mutex> lock{_mutex};

    if (!_msg)
        _cond.wait(lock);

    if (_msg & MSG_SET_WINDOW)
        *window = _window;
    msg_orig = _msg;
    _msg = 0;

    return msg_orig;
}

void Renderer::resume()
{
    pr_info();
    push(MSG_RESUME, 0);
}

void Renderer::pause()
{
    pr_info();
    push(MSG_PAUSE, 0);
}

void Renderer::setWindow(ANativeWindow *window)
{
    pr_info();
    push(MSG_SET_WINDOW, window);
}

void Renderer::changeContext(void)
{
    pr_info();
    push(MSG_CHANGE_CONTEXT, 0);
}

void Renderer::threadFunc(Renderer *renderer)
{
    bool run = true;
    bool resumed = false;

    struct es2gears_state *uhmi_state = 0;
    bool uhmi_is_running = false;

    struct es2gears_state *android_state = 0;
    bool android_is_running = false;

    typedef std::chrono::high_resolution_clock Time;
    auto start_time = Time::now();

    pr_info("START");

    while (run) {

        int msg;
        int err;
        ANativeWindow* window;

        if (uhmi_is_running | android_is_running)
            msg = renderer->pop(&window);
        else
            msg = renderer->popWait(&window);

        if (msg & MSG_SET_WINDOW) {

            pr_info("MSG_SET_WINDOW");

            uhmi_is_running = false;
            android_is_running = false;

            if (android_state) {

                libuhmigl_android_load();

                es2gears_done(android_state);
                libuhmigl_android_done();
                android_state = 0;
            }

            uint16_t h, v;
            libuhmigl_android_init(window, &h, &v);
            android_state = es2gears_init();
            es2gears_reshape(android_state, h, v);

            if (resumed)
                android_is_running = true;

        }

        if (msg & MSG_EXIT) {

            pr_info("MSG_EXIT");

            android_is_running = false;
            uhmi_is_running = false;
            run = false;

        }

        if (msg & MSG_RESUME) {

            pr_info("MSG_RESUME");

            resumed = true;

            if (android_state)
                android_is_running = true;
        }

        if (msg & MSG_PAUSE) {

            pr_info("MSG_PAUSE");

            resumed = false;
            uhmi_is_running    = false;
            android_is_running = false;
        }

        if (msg & MSG_CHANGE_CONTEXT) {

            pr_info("MSG_CHANGE_CONTEXT");

            if (android_is_running) {

                android_is_running = false;

                if (!uhmi_state) {
                    uint16_t h, v;
                    libuhmigl_init(&h, &v);
                    uhmi_state = es2gears_init();
                    es2gears_reshape(uhmi_state, h, v);
                } else {
                    libuhmigl_load();
                }

                if (resumed)
                    uhmi_is_running = true;

            } else if (uhmi_is_running && android_state) {

                uhmi_is_running = false;

                libuhmigl_android_load();

                if (resumed)
                    android_is_running = true;
            }
        }

        if (android_is_running) {

            assert(android_state);

            auto now = Time::now();
            auto delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

            es2gears_idle(android_state, delta_ms.count());
            es2gears_draw(android_state);

            int err = libuhmigl_android_update();
            if (err) {
                pr_err("libuhmigl_android_update() returned error %d", err);
                android_is_running = false;
            }
        }

        if (uhmi_is_running) {

            assert(uhmi_state);

            auto now = Time::now();
            auto delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

            es2gears_idle(uhmi_state, delta_ms.count());
            es2gears_draw(uhmi_state);

            err = libuhmigl_update();
            if (err) {
                pr_err("libuhmigl_update() returned error %d", err);
                uhmi_is_running = false;
            }

        }

    }

    if (uhmi_state) {
        es2gears_done(uhmi_state);
        libuhmigl_done();
    }
    if (android_state) {
        es2gears_done(android_state);
        libuhmigl_android_done();
    }

    pr_info("STOP");
}

