
#include "char_select.hpp"

CharSelect::CharSelect()
: m_databaseThread(m_logWriter),
  m_database(m_databaseThread, m_logWriter),
  m_logWriter(SourceId::CharSelect, m_ipc)
{
    
}

void CharSelect::init()
{
    // Timer pool and timers
    m_timerPool.init();
    
    // Database thread and main database file
    m_databaseThread.init();
    m_database.init(EQP_SQLITE_MAIN_DATABASE_PATH, EQP_SQLITE_MAIN_SCHEMA_PATH);
}
