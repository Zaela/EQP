
#include "master.hpp"

extern std::atomic_bool g_shutdown;

Master::Master()
: m_databaseThread(m_logWriter),
  m_database(m_databaseThread, m_logWriter),
  m_ipcCharSelect(m_ipcSemaphore),
  m_ipcLogin(m_ipcSemaphore),
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
    m_logWriter.openLogFileFor(SourceId::CharSelect);
    
    spawnProcess(EQP_CHAR_SELECT_BIN, EQP_IPC_CHAR_SELECT);
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
        
        if (g_shutdown)
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
        
        // IPC output queues for pushes outside the IPC thread, or for overflows
        m_ipcCharSelect.processOutQueue();
        m_ipcLogin.processOutQueue();
        
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
    
    case ServerOp::LoginClientAuth:
        // Forward to CharSelect
        m_ipcCharSelect.forward(packet);
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
    
    int sourceId        = packet.sourceId();
    uint32_t accountId  = req->accountId;
    uint32_t serverId   = req->serverId;
    
    Query query;
    
    m_database.prepare(query, "SELECT status, (suspended_until > strftime('%s', 'now')) FROM account WHERE login_server_id = ?",
    [this, sourceId, accountId, serverId](Query& query)
    {
        int respValue = 1;
        
        while (query.select())
        {
            // Are they banned or suspended?
            if (query.getInt(1) < 0)
                respValue = -2;
            else if (query.getInt(2) > 0)
                respValue = -1;
        }
        
        LoginStruct::Response resp;
    
        resp.accountId  = accountId;
        resp.serverId   = serverId;
        resp.response   = (int8_t)respValue;
        
        if (sourceId == SourceId::CharSelect)
            m_ipcCharSelect.pushThreadSafe(ServerOp::LoginResponse, sourceId, sizeof(LoginStruct::Response), &resp);
        else
            m_ipcLogin.pushThreadSafe(ServerOp::LoginResponse, sourceId, sizeof(LoginStruct::Response), &resp);
    });
    
    m_database.schedule(query);
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

void Master::shutDownChildProcesses()
{
    m_ipcCharSelect.push(ServerOp::Shutdown, SourceId::Master, 0, nullptr);
    m_ipcLogin.push(ServerOp::Shutdown, SourceId::Master, 0, nullptr);
    
    Clock::sleepMilliseconds(250);
}
