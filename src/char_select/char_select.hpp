
#ifndef _EQP_CHAR_SELECT_HPP_
#define _EQP_CHAR_SELECT_HPP_

#include "define.hpp"
#include "timer_pool.hpp"
#include "database.hpp"
#include "log_writer_common.hpp"
#include "ipc_buffer.hpp"
#include "source_id.hpp"

// CharSelect is comprised of 3 threads:
// the main thread, which runs callbacks, handles IPC input, and handles UDP output (?)
// the database thread;

class CharSelect
{
private:
    TimerPool       m_timerPool;
    DatabaseThread  m_databaseThread;
    Database        m_database;
    LogWriterCommon m_logWriter;
    IpcRemote       m_ipc;
    
public:
    CharSelect();

    void init(const char* ipcPath);
    void mainLoop();
};

#endif//_EQP_CHAR_SELECT_HPP_
