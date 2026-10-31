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
#include "Async.h"
#include <FL/Fl.H>

static void AwakeCallback(void *data)
{
    std::function<void()> *function=(std::function<void()>*)data;
    (*function)();
    delete function;
}

void RunInUiThread(std::function<void()> function)
{
    Fl::awake(AwakeCallback, new std::function<void()>(function));
}

TaskPool::TaskPool(int threads)
{
    for(int i=0; i<threads; i++)
    {
        Threads.push_back(std::thread(&TaskPool::Worker, this));
        Threads.back().detach(); //The pools live as long as the program
    }
}

void TaskPool::Run(std::function<void()> work, std::function<void()> done)
{
    {
        std::lock_guard<std::mutex> lock(Lock);
        Tasks.push(std::make_pair(work, done));
    }
    Wake.notify_one();
}

void TaskPool::ClearPending()
{
    std::lock_guard<std::mutex> lock(Lock);
    while(!Tasks.empty())
        Tasks.pop();
}

void TaskPool::Worker()
{
    while(true)
    {
        std::pair< std::function<void()>, std::function<void()> > task;
        {
            std::unique_lock<std::mutex> lock(Lock);
            Wake.wait(lock, [this](){ return !Tasks.empty(); });
            task=Tasks.front();
            Tasks.pop();
        }
        task.first();
        if(task.second)
            RunInUiThread(task.second);
    }
}
