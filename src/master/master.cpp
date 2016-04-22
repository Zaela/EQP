
#include "master.hpp"

Master::Master()
: m_databaseThread(m_logWriter),
  m_database(m_databaseThread, m_logWriter)
{
    
}

void Master::init()
{
    // Start the log writer first
    m_logWriter.init();
    
    // Timer pool and timers
    m_timerPool.init();
    
    // Database thread and main database file
    m_databaseThread.init();
    m_database.init(EQP_SQLITE_MAIN_DATABASE_PATH, EQP_SQLITE_MAIN_SCHEMA_PATH);
    
    // IPC
    m_ipcSemaphore.init();
    
    m_ipcLogin.init(EQP_IPC_LOGIN);
    m_ipcCharSelect.init(EQP_IPC_CHAR_SELECT);
}

/*================================================================================*\
** Convenience methods
\*================================================================================*/

void Master::log(Log::Type type, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    m_logWriter.log(type, fmt, args);
    va_end(args);
}

/*================================================================================*\
** Other
\*================================================================================*/

void Master::executeBackgroundThreadCallbacks()
{
    m_timerPool.executeTimerCallbacks();
    m_databaseThread.executeQueryCallbacks();
}

pid_t Master::spawnProcess(const char* path, const char* arg)
{
    pid_t pid = fork();
    
    if (pid == 0)
    {
        const char* argv[] = {path, arg, nullptr};
        
        if (execv(path, (char**)argv))
        {
            throw Exception("[Master::spawnProcess] child process execv() failed attempting to execute '%s', aborting", path);
        }
    }
    else if (pid < 0)
    {
        // Fork failed
        throw Exception("[Master::spawnProcess] fork() failed");
    }
    
    return pid;
}
