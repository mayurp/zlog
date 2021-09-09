//
//  logger.cpp
//  logger
//
//  Created by Mayur Patel on 09/09/2021.
//

#include "logger.hpp"

#include "barectf-platform-linux-fs.h"
#include <exception>
#include <thread>
#include <sstream>


namespace barectf
{

thread_local barectf_default_ctx* current_thread_context = nullptr;
std::atomic_int thread_id = 0;

barectf_default_ctx* get_context()
{
    return current_thread_context;
}

ScopedContext::ScopedContext()
{
    if (current_thread_context)
        std::logic_error("Bare ctf context already initialised on this thread");
    
    // TODO: pass trace file path in separately
    std::ostringstream ss;
    ss << "trace/stream_" << thread_id++;
    platform_ctx = barectf_platform_linux_fs_init(512, ss.str().c_str(), 0, 0, 0);
    current_thread_context = barectf_platform_linux_fs_get_barectf_ctx(platform_ctx);
}

ScopedContext::~ScopedContext()
{
    current_thread_context = nullptr;
    barectf_platform_linux_fs_fini(platform_ctx);
    platform_ctx = nullptr;
}



}
