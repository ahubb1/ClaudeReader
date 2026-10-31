//--------------------------------------------------------------------------
//
//    This file is part of ClaudeReader (based on BDReader, Copyright (C) 2014-2016  Vincent Crocher).
//
//    ClaudeReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ClaudeReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ClaudeReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file Async.h
 * \brief Background work for the user interface: tasks run in worker threads, their result is
 * handled in the user interface thread (FLTK must be initialized with Fl::lock()).
 */
#ifndef ASYNC_H
#define ASYNC_H

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

//! Fixed number of worker threads executing queued tasks
class TaskPool
{
    public:
        TaskPool(int threads);
        //! Run work() in a worker thread, then done() in the user interface thread
        void Run(std::function<void()> work, std::function<void()> done=std::function<void()>());
        //! Forget the tasks not started yet
        void ClearPending();

    private:
        void Worker();
        std::mutex Lock;
        std::condition_variable Wake;
        std::queue< std::pair< std::function<void()>, std::function<void()> > > Tasks;
        std::vector<std::thread> Threads;
};

//! Run a function in the user interface thread (from any thread)
void RunInUiThread(std::function<void()> function);

#endif // ASYNC_H
