#include "HTTPSessionsCleaner.h"

#include <Poco/Util/Application.h>
#include <Poco/File.h>

#include <common/logger_useful.h>

#include <Interpreters/Context.h>
#include <Common/setThreadName.h>
#include <Common/ConfigProcessor.h>

namespace DB {

HTTPSessionsCleaner::HTTPSessionsCleaner(Context *global_context_) : global_context(global_context_)
{
    thread = std::thread(&HTTPSessionsCleaner::run, this);
}

HTTPSessionsCleaner::~HTTPSessionsCleaner()
{
    quit = true;
    thread.join();
}

void HTTPSessionsCleaner::run()
{
    setThreadName("HTTPSessionsCleaner");

    while (true) {
        CleanSessions();
        if (quit)
            return;
        //TODO set this value in xml
        std::this_thread::sleep_for(std::chrono::seconds(thread_sleep_time));
    }
}

void HTTPSessionsCleaner::CleanSessions()
{
    auto lock = global_context->getLock();

    std::vector <std::pair<std::string, std::string>> sessions_to_delete;

    auto session_map = global_context->GetSessionsMap();
    for (auto user = session_map->begin(); user != session_map->end(); ++user) {
        for (auto session = user->second.begin(); session != user->second.end(); ++session) {

            LOG_DEBUG(log, "Session_expired_state is   " << session->second->SessionExpired());
            LOG_DEBUG(log, "Shared ptr usage flag is   " << session->second.unique());

            if ((session->second->SessionExpired()) && (session->second.unique())) {
                sessions_to_delete.emplace_back(user->first, session->first);
            }
        }
    }

    for (const auto &ids : sessions_to_delete) {
        session_map->at(ids.first).erase(ids.second);
    }
}

}