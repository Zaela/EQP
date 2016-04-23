
#include "define.hpp"
#include "terminal.hpp"
#include "log_client.hpp"
#include "master.hpp"
#include "clock.hpp"
#include <exception>
#include <thread>
#include <chrono>

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;
    
    Master master;
    
    try
    {
        master.init();
        master.mainLoop();
    }
    catch (std::exception& e)
    {
        printf(
            TERM_RED "[ERROR]\n"
            "================================================================================\n" TERM_DEFAULT
            TERM_YELLOW "%s\n"
            TERM_RED "================================================================================\n" TERM_DEFAULT,
            e.what());
        
        master.logWriter().log(Log::Fatal, "%s", e.what());
    }
    catch (...)
    {
        printf(
            TERM_RED "[ERROR]\n"
            "================================================================================\n" TERM_DEFAULT
            TERM_YELLOW "Caught unknown exception...\n"
            TERM_RED "================================================================================\n" TERM_DEFAULT);
    }
    
    // Give threads a little time to have their resourced cleaned up;
    // may get mem leak false positives from Valgrind otherwise
    Clock::sleepMilliseconds(25);
    
    return 0;
}
