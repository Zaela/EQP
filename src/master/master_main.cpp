
#include "define.hpp"
#include "terminal.hpp"
#include "master.hpp"
#include "clock.hpp"
#include <exception>

std::atomic_bool g_shutdown;

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;
    
    g_shutdown = false;
    
    signal(SIGINT, [](int)
    {
        printf("\n--Interrupted!\n");
        g_shutdown = true;
    });
    
    // Scope for the Master object
    {
        Master master;
    
        try
        {
            master.init();
            master.mainLoop();
            master.shutDownChildProcesses();
            master.logWriter().log(Log::Info, "Shutting down cleanly");
        }
        catch (std::exception& e)
        {
            std::lock_guard<AtomicMutex> lock(master.logWriter().printfMutex());
            
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
    }
    
    // Give threads a little time to have their resourced cleaned up;
    // may get mem leak false positives from Valgrind otherwise
    Clock::sleepMilliseconds(25);
    
    return 0;
}
