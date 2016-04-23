
#include "master.hpp"

Master::Master()
: m_databaseThread(m_logWriter),
  m_database(m_databaseThread, m_logWriter),
  m_mainLoopEnd(false),
  m_ipcThreadEnd(false)
{
    
}

Master::~Master()
{
    m_ipcThreadEnd = true;
    m_ipcSemaphore.trigger();
    std::lock_guard<AtomicMutex> lock(m_ipcThreadLifetimeMutex);
}

/*================================================================================*\
** Initialization
\*================================================================================*/

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
    
    std::thread ipcThread(ipcThreadProc, this);
    ipcThread.detach();
    
    launchCharSelect();
    launchLogin();
}

void Master::launchCharSelect()
{
    m_ipcCharSelect.init(EQP_IPC_CHAR_SELECT);
    //m_logWriter.openLogFileFor(SourceId::CharSelect);
    
    //spawnProcess(EQP_CHAR_SELECT_BIN, EQP_IPC_CHAR_SELECT);
}

void Master::launchLogin()
{
    m_ipcLogin.init(EQP_IPC_LOGIN);
    m_logWriter.openLogFileFor(SourceId::Login);
    
    spawnProcess(EQP_LOGIN_BIN, EQP_IPC_LOGIN, "EQP Test"); //fixme: get server name from config
}

/*================================================================================*\
** Main thread
\*================================================================================*/

void Master::mainLoop()
{
    for (;;)
    {
        m_timerPool.executeTimerCallbacks();
        m_databaseThread.executeQueryCallbacks();
        
        // IPC output queues for pushes outside the IPC thread, or for overflows
        m_ipcLogin.processOutQueue();
        m_ipcCharSelect.processOutQueue();
        
        if (m_mainLoopEnd)
            return;
        
        Clock::sleepMilliseconds(50);
    }
}

/*================================================================================*\
** IPC thread
\*================================================================================*/

void Master::ipcThreadProc(Master* master)
{
    std::lock_guard<AtomicMutex> lock(master->m_ipcThreadLifetimeMutex);
    master->ipcThreadLoop();
}

void Master::ipcThreadLoop()
{
    m_logWriter.log(Log::Info, "IPC Thread started");
    
    for (;;)
    {
        m_ipcSemaphore.wait();
        
        processIpcInput(m_ipcCharSelect);
        processIpcInput(m_ipcLogin);
        
        if (m_ipcThreadEnd)
            return;
    }
}

void Master::processIpcInput(IpcMaster& ipc)
{
    for (;;)
    {
        IpcPacket packet;
        
        if (!ipc.pop(packet))
            return;
        
        processIpcInput(packet);
    }
}

void Master::processIpcInput(IpcPacket& packet)
{
    switch (packet.opcode())
    {
    // Common
    case ServerOp::LogMessage:
        m_logWriter.log(packet);
        break;
    
    // ZoneCluster
    // Zone
    // CharSelect
    // Login
    case ServerOp::LoginRequest:
        validateLoginRequest(packet);
        break;
    
    default:
        break;
    }
}

/*================================================================================*\
** IPC handlers
\*================================================================================*/

void Master::validateLoginRequest(IpcPacket& packet)
{
    if (packet.length() < sizeof(LoginStruct::Request))
        return;
    
    LoginStruct::Request* req = (LoginStruct::Request*)packet.data();
    
    //do validation here
    
    LoginStruct::Response resp;
    
    resp.accountId  = req->accountId;
    resp.serverId   = req->serverId;
    resp.response   = 1;
    
    if (packet.sourceId() == SourceId::CharSelect)
        m_ipcCharSelect.push(ServerOp::LoginResponse, packet.sourceId(), sizeof(LoginStruct::Response), &resp);
    else
        m_ipcLogin.push(ServerOp::LoginResponse, packet.sourceId(), sizeof(LoginStruct::Response), &resp);
}

/*================================================================================*\
** Other
\*================================================================================*/

pid_t Master::spawnProcess(const char* path, const char* arg1, const char* arg2)
{
    m_logWriter.log(Log::Info, "Spawning process \"%s\" with args \"%s\", \"%s\"", path, arg1 ? arg1 : "(null)", arg2 ? arg2 : "(null)");
    
    pid_t pid = fork();
    
    if (pid == 0)
    {
        const char* argv[] = {path, arg1, arg2, nullptr};
        
        if (execv(path, (char**)argv))
        {
            // This will be caught by the main() of the forked Master child process.. yeah
            throw Exception("[Master::spawnProcess] child process execv() failed attempting to execute '%s', aborting", path);
        }
    }
    else if (pid < 0)
    {
        // Fork failed
        throw Exception("[Master::spawnProcess] fork() failed");
    }
    
    m_logWriter.log(Log::Info, "Spawned process \"%s\" with pid %i", path, pid);
    
    return pid;
}
