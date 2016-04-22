
#ifndef _EQP_MASTER_HPP_
#define _EQP_MASTER_HPP_

#include "define.hpp"
#include "database.hpp"
#include "timer_pool.hpp"
#include "log_writer_master.hpp"
#include "ipc_master.hpp"
#include "master_semaphore_creator.hpp"
#include "exception.hpp"

#ifdef EQP_LINUX
# include <sys/types.h>
# include <unistd.h>
#endif

class Master
{
private:
    TimerPool       m_timerPool;
    DatabaseThread  m_databaseThread;
    Database        m_database;
    LogWriterMaster m_logWriter;

    // IPC
    MasterSemaphoreCreator  m_ipcSemaphore;
    
    IpcMaster m_ipcLogin;
    IpcMaster m_ipcCharSelect;

public:
    Master();

    void init();
    
    // Object accessors
    Database& database() { return m_database; }
    
    // Convenience methods
    void log(Log::Type type, const char* fmt, ...);
    
    // Other
    void executeBackgroundThreadCallbacks();
    
    pid_t spawnProcess(const char* path, const char* arg = nullptr);
};

#endif//_EQP_MASTER_HPP_
