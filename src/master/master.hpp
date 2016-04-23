
#ifndef _EQP_MASTER_HPP_
#define _EQP_MASTER_HPP_

#include "define.hpp"
#include "database.hpp"
#include "timer_pool.hpp"
#include "log_writer_master.hpp"
#include "ipc_master.hpp"
#include "master_semaphore_creator.hpp"
#include "exception.hpp"
#include "source_id.hpp"
#include "server_op.hpp"
#include "atomic_mutex.hpp"
#include "clock.hpp"
#include "packet_structs_login.hpp"

#ifdef EQP_LINUX
# include <sys/types.h>
# include <unistd.h>
#endif

#ifdef EQP_WINDOWS
# define EQP_CHAR_SELECT_BIN    "eqp-char-select.exe"
# define EQP_LOGIN_BIN          "eqp-login.exe"
#else
# define EQP_LOGIN_BIN          "./eqp-login"
# define EQP_CHAR_SELECT_BIN    "./eqp-char-select"
#endif

// Master is comprised of 4 threads:
// the main thread, which just runs timers and db query callbacks and sleeps comfortably in between (50 ms);
// the database thread;
// the logging thread;
// and the IPC thread, which spends most of its time blocking on the master semaphore
//
// Most data manipulation happens in the IPC thread, in direct response to requests;
// for this reason query callbacks, which happen in the main thread, must synchronize
// before manipulating any data that belongs to Master.

class Master
{
private:
    TimerPool           m_timerPool;
    DatabaseThread      m_databaseThread;
    Database            m_database;
    LogWriterMaster     m_logWriter;

    std::atomic_bool    m_mainLoopEnd;

    // IPC
    MasterSemaphoreCreator  m_ipcSemaphore;
    
    IpcMaster m_ipcCharSelect;
    IpcMaster m_ipcLogin;

    std::atomic_bool    m_ipcThreadEnd;
    AtomicMutex         m_ipcThreadLifetimeMutex;

private:
    void launchCharSelect();
    void launchLogin();

    void ipcThreadLoop();
    static void ipcThreadProc(Master* master);
    void processIpcInput(IpcMaster& ipc);
    void processIpcInput(IpcPacket& packet);

    // IPC handlers
    void validateLoginRequest(IpcPacket& packet);

    pid_t spawnProcess(const char* path, const char* arg1 = nullptr, const char* arg2 = nullptr);

public:
    Master();
    ~Master();

    void init();
    void mainLoop();

    LogWriterMaster& logWriter() { return m_logWriter; }
};

#endif//_EQP_MASTER_HPP_
