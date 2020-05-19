#include "skyline/logger/Logger.hpp"

#include "operator.h"
#include "alloc.h"
#include "mem.h"

#include <cstdarg>

namespace skyline::logger {

    Logger* s_Instance;

    std::queue<char*>* g_msgQueue = nullptr;
    
    void ThreadMain(void* arg) {
#ifndef NDEBUG
        Logger* t = (Logger*) arg;

        t->Initialize();

        t->LogFormat("[%s] Logger initialized.", t->FriendlyName().c_str());

        while(true){
            t->Flush();
            nn::os::YieldThread(); // let other parts of OS do their thing
            nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(100000000));
        }
#endif
    }

    void Logger::StartThread(){
#ifndef NDEBUG
        const size_t stackSize = 0x3000;
        void* threadStack = memalign(0x1000, stackSize);
        
        nn::os::ThreadType* thread = new nn::os::ThreadType;
        nn::os::CreateThread(thread, ThreadMain, this, threadStack, stackSize, 16, 0);
        nn::os::StartThread(thread);
#endif
    }

    void Logger::SendRaw(const char* data)
    {
#ifndef NDEBUG
        SendRaw((void*)data, strlen(data));
#endif
    }

    void Logger::SendRawFormat(const char* format, ...)
    {
#ifndef NDEBUG
        va_list args;
        char buff[0x1000] = {0};
        va_start(args, format);

        int len = vsnprintf(buff, sizeof(buff), format, args);
        
        SendRaw(buff, len);
        
        va_end (args);
#endif
    }

    void AddToQueue(char* data)
    {
#ifndef NDEBUG
        if(!g_msgQueue)
            g_msgQueue = new std::queue<char*>();

        g_msgQueue->push(data);
#endif
    }
    
    void Logger::Flush() {
#ifndef NDEBUG
        if(!g_msgQueue)
            return;

        while (!g_msgQueue->empty())
        {
            auto data = g_msgQueue->front();

            SendRaw(data, strlen(data));
            delete[] data;
            g_msgQueue->pop();
        }
#endif
    }

    void Logger::Log(const char* data, size_t size)
    {
#ifndef NDEBUG
        if (size == UINT32_MAX)
            size = strlen(data);

        char* ptr = new char[size+2];
        memset(ptr, 0, size+2);
        memcpy(ptr, data, size);
        //ptr[size] = '\n';

        AddToQueue(ptr);
        return;
#endif
    }

    void Logger::Log(std::string str)
    {
#ifndef NDEBUG
        Log(str.data(), str.size());
#endif
    }

    void Logger::LogFormat(const char* format, ...)
    {
#ifndef NDEBUG
        va_list args;
        va_start(args, format);

        size_t len = vsnprintf(NULL, 0, format, args);
        char* ptr = new char[len+2];
        memset(ptr, 0, len+2);
        vsnprintf(ptr, len+1, format, args);
        ptr[len] = '\n';
         
        AddToQueue(ptr);
        va_end (args);

        return;
#endif
    }
};