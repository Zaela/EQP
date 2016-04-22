
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
    int source = std::stoi(argv[1]);
    
    Master master;
    
    try
    {
        master.init();
        
        Query q;
        
        master.database().prepare(q, "SELECT x FROM test WHERE rowid < (? * ?)", [](Query& query)
        {
            printf("=====================\n");
            while (query.select())
            {
                printf("%li\n", query.getInt64(1));
            }
        });
        
        q.bindInt(1, source);
        q.bindInt(2, source);
        master.database().schedule(q);
        
        master.database().prepare(q, "SELECT x FROM test WHERE rowid >= (? * 2)", [](Query& query)
        {
            printf("=====================\n");
            while (query.select())
            {
                int64_t val = query.getInt64(1);
                
                printf("selected %li\n", val);
            }
        });
        
        q.bindInt(1, source);
        
        master.database().schedule(q);
        
        master.database().prepareAndSchedule("SELECT MAX(x), MIN(x), COUNT(x) FROM test", [](Query& query)
        {
            while (query.select())
            {
                printf("max: %li, min: %li, count: %i\n",
                    query.getInt64(1), query.getInt64(2), query.getInt(3));
            }               
        });

        LogClient log;
        char num[7];
        
        master.spawnProcess("./eqp-log");
        Clock::sleepMilliseconds(100);
        
        for (int i = 0; i < 100000; i++)
        {
            sprintf(num, "%06i", i);
            log.push(source, num, 6);
            //log.push(SourceId::Master, argv[1], len);
        }
        
        log.informServer();
        
        if (log.hasPending())
            printf("PENDING\n");
        
        while (log.hasPending())
        {
            //std::this_thread::sleep_for(std::chrono::milliseconds(1));
            log.pushPending();
            log.informServer();
        }
        
        log.informServer();
        Clock::sleepMilliseconds(1000);
        log.sendExitSignalToServer();
        
        master.executeBackgroundThreadCallbacks();
    }
    catch (int& e)
    {
        printf("Exception: %i\n", e);
    }
    catch (std::exception& e)
    {
        printf(
            TERM_RED "[ERROR]\n"
            "================================================================================\n" TERM_DEFAULT
            TERM_YELLOW "%s\n"
            TERM_RED "================================================================================\n" TERM_DEFAULT,
            e.what());
        
        master.log(Log::Fatal, "%s", e.what());
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
