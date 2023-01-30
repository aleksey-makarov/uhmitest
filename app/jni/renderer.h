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

#ifndef RENDERER_H
#define RENDERER_H

#include <mutex>
#include <condition_variable>
#include <thread>

struct ANativeWindow;

class Renderer {

public:
    Renderer();
    virtual ~Renderer();

    // Following methods can be called from any thread.
    // They send message to render thread which executes required actions.
    void resume();
    void pause();
    void setWindow(ANativeWindow* window);
    void changeContext();

private:

    void push(int msg, ANativeWindow *window);
    int pop(ANativeWindow **window);
    int popWait(ANativeWindow **window);

    // Helper method for starting the thread 
    static void threadFunc(Renderer *renderer);

    enum RenderThreadMessage {
        MSG_EXIT           = 1 << 0,
        MSG_RESUME         = 1 << 1,
        MSG_PAUSE          = 1 << 2,
        MSG_SET_WINDOW     = 1 << 3,
        MSG_CHANGE_CONTEXT = 1 << 4,
    };

    int _msg;
    ANativeWindow *_window;

    std::mutex _mutex;
    std::condition_variable _cond;
    std::thread _thread;
};

#endif // RENDERER_H
