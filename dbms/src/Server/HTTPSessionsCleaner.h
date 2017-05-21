#pragma once

#include <Common/ConfigProcessor.h>

#include <time.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <list>

namespace Poco { class Logger; }

namespace DB
{

class Context;


class HTTPSessionsCleaner
{
public:
    HTTPSessionsCleaner(Context* global_context_);
    ~HTTPSessionsCleaner();
private:
    void run();
    void CleanSessions();
    std::atomic<bool> quit{false};
    std::thread thread;
    uint32_t thread_sleep_time = 1;

    Context* global_context;

    Poco::Logger * log = &Logger::get("HTTPSessionsCleaner");
};

}