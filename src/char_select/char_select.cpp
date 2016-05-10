
#include "char_select.hpp"

CharSelect::CharSelect()
: m_shutdown(false),
  m_databaseThread(m_logWriter),
  m_database(m_databaseThread, m_logWriter),
  m_logWriter(SourceId::CharSelect, m_ipc),
  m_socket(m_logWriter)
{
    
}

void CharSelect::init(const char* ipcPath)
{
    // Shared Memory IPC area
    // Must be done before anything that tries to log, since log messages go through IPC
    m_ipc.init(ipcPath);
    
    // Timer pool and timers
    m_timerPool.init();
    
    // Database thread and main database file
    m_databaseThread.init();
    m_database.init(EQP_SQLITE_MAIN_DATABASE_PATH, EQP_SQLITE_MAIN_SCHEMA_PATH);
    
    // Open socket
    m_socket.open(EQP_CHAR_SELECT_PORT);
}

void CharSelect::mainLoop()
{
    for (;;)
    {
        handleIpc();
        
        m_socket.receive();
        m_socket.processPacketQueues();
        
        m_timerPool.executeTimerCallbacks();
        m_databaseThread.executeQueryCallbacks();
        
        if (m_shutdown)
            return;
        
        Clock::sleepMilliseconds(25);
    }
}

void CharSelect::handleIpc()
{
    // Check pending IPC output
    m_ipc.processOutQueue();
    
    // Check IPC input
    for (;;)
    {
        IpcPacket packet;
        
        if (!m_ipc.pop(packet))
            break;
        
        processIpc(packet);
    }
}

void CharSelect::processIpc(IpcPacket& packet)
{
    switch (packet.opcode())
    {
    case ServerOp::Shutdown:
        m_shutdown = true;
        m_logWriter.log(Log::Info, "Received shutdown command from Master");
        break;
    
    case ServerOp::LoginClientAuth:
        // Auth from the localhost login server
        handleClientAuth(packet.data(), packet.length());
        break;
    
    default:
        break;
    }
}

void CharSelect::handleClientAuth(byte* data, uint32_t len)
{
    if (len != sizeof(LoginStruct::ClientAuth))
        return;
    
    AlignedReader r(data, len);
    
    UdpSocket::Authorized a;
    
    // Account ID
    a.accountId = r.uint32();
    // Skip name
    r.advance(30);
    // Session Key
    r.buffer(a.sessionKey, 10);
    // Unused bytes of Session Key, plus loginAdminLevel and worldAdminLevel
    r.advance(20 + sizeof(uint8_t) + sizeof(int16_t));
    // IP address
    a.ipAddress = r.uint32();
    
    uint32_t ip = a.ipAddress;
    printf("Received auth for %u.%u.%u.%u account %u\n", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, a.accountId);
    
    m_socket.addClientAuth(a);
}
