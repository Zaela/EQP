
#ifndef _EQP_CHAR_SELECT_HPP_
#define _EQP_CHAR_SELECT_HPP_

#include "define.hpp"
#include "timer_pool.hpp"
#include "database.hpp"
#include "log_writer_common.hpp"
#include "ipc_buffer.hpp"
#include "source_id.hpp"
#include "char_select_socket.hpp"
#include "clock.hpp"
#include "packet_structs_login.hpp"
#include "aligned.hpp"

// CharSelect is comprised of 3 threads:
// the main thread, which runs callbacks, handles IPC input, and handles UDP output (?)
// the database thread;

//charselect needs to liase with login via tcp

#define EQP_CHAR_SELECT_PORT 9000

class CharSelect
{
private:
    bool                m_shutdown;
    TimerPool           m_timerPool;
    DatabaseThread      m_databaseThread;
    Database            m_database;
    LogWriterCommon     m_logWriter;
    IpcRemote           m_ipc;
    CharSelectSocket    m_socket;

private:
    void handleIpc();
    void processIpc(IpcPacket& packet);

    void handleClientAuth(byte* data, uint32_t len);
    
public:
    CharSelect();

    void init(const char* ipcPath);
    void mainLoop();
};

#endif//_EQP_CHAR_SELECT_HPP_
