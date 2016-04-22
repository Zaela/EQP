
#include "define.hpp"
#include "log_server.hpp"
#include "terminal.hpp"
#include "clock.hpp"
#include <exception>
#include <thread>
#include <chrono>

int main()
{
    try
    {
        LogServer server;
        server.startThread();
        server.mainLoop();
    }
    catch (std::exception& e)
    {
        printf(
            TERM_RED "[ERROR]\n"
            "================================================================================\n" TERM_DEFAULT
            TERM_YELLOW "%s\n"
            TERM_RED "================================================================================\n" TERM_DEFAULT,
            e.what());
    }
    catch (...)
    {
        printf(TERM_RED "[ERROR] Caught unknown exception...\n" TERM_DEFAULT);
    }
    
    // Give threads a little time to have their resourced cleaned up;
    // may get mem leak false positives from Valgrind otherwise
    Clock::sleepMilliseconds(25);
    
    return 0;
}
