
#include "database.hpp"

Database::Database(DatabaseThread& dbThread, LogWriter& logWriter)
: m_sqlite(nullptr),
  m_thread(dbThread),
  m_logWriter(logWriter),
  m_queryNum(0)
{
    
}

Database::~Database()
{
    sqlite3_close_v2(m_sqlite);
}

void Database::init(const char* dbPath, const char* schemaPath)
{
    int rc = sqlite3_open_v2(
        dbPath,
        &m_sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
        nullptr
    );
    
    if (rc != SQLITE_OK)
    {
        if (rc != SQLITE_CANTOPEN)
            throw Exception("[Database::init] Could not open database file '%s'\nReason: %s", dbPath, sqlite3_errstr(rc));
        
        createFromSchema(dbPath, schemaPath);
    }
    
    m_dbPath = dbPath;
    m_logWriter.log(Log::Info, "Database file '%s' opened and initialized", dbPath);
}

void Database::createFromSchema(const char* dbPath, const char* schemaPath)
{
    int rc = sqlite3_open_v2(
        dbPath,
        &m_sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE,
        nullptr
    );
    
    if (rc != SQLITE_OK)
        throw Exception("[Database::createFromSchema] Could not create database file '%s'\nReason: %s", dbPath, sqlite3_errstr(rc));
    
    if (!schemaPath)
        return;
    
    File schema(schemaPath);
    
    if (schema.exists())
    {
        m_logWriter.log(Log::Info, "Database file '%s' did not exist, creating from schema file '%s'", dbPath, schemaPath);
        exec(schema.readAll(), "[Database::createFromSchema] Error running schema file\nReason: %s");
    }
}

void Database::prepare(Query& query, const std::string& sql, Query::Callback callback)
{
    PerfTimer timer;
    
    if (callback)
        query.setCallback(callback);
    
    query.setDb(this);
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_sqlite, sql.c_str(), sql.length(), &stmt, nullptr);
    
    if (rc == SQLITE_OK)
    {
        query.setStmt(stmt);
        m_logWriter.log(Log::SQL, "Prepared query %u against database '%s' in %llu microseconds. SQL: \"%s\"",
            m_queryNum, m_dbPath.c_str(), timer.microseconds(), sql.c_str());
        return;
    }
    
    throw Exception("[Database::prepare] Could not prepare query\nReason: %s\nSQL: %s", sqlite3_errstr(rc), sql.c_str());
}

void Database::beginTransaction()
{
    exec("BEGIN", "[Database::beginTransaction] %s");
}

void Database::commitTransaction()
{
    exec("COMMIT", "[Database::commitTransaction] %s");
}

void Database::exec(const std::string& sql, const char* exceptionFormat)
{
    char* errmsg = nullptr;
    
    int rc = sqlite3_exec(m_sqlite, sql.c_str(), nullptr, nullptr, &errmsg);
    
    if (errmsg)
    {
        char buf[Exception::MAX_LENGTH];
        ::snprintf(buf, Exception::MAX_LENGTH, "%s", errmsg);
        sqlite3_free(errmsg);
        
        if (exceptionFormat)
            throw Exception(exceptionFormat, buf);
        
        throw Exception("[Database::exec] Could not complete query\nReason: %s", buf);
    }
    
    // Is it even possible to get this without errmsg being filled?
    if (rc != SQLITE_OK)
    {
        if (exceptionFormat)
            throw Exception(exceptionFormat, sqlite3_errstr(rc));
        
        throw Exception("[Database::exec] Could not complete query\nReason: %s", sqlite3_errstr(rc));
    }
}

void Database::schedule(Query& query)
{
    m_thread.schedule(query);
}

void Database::prepareAndSchedule(const std::string& sql, Query::Callback callback)
{
    Query query;
    prepare(query, sql, callback);
    m_thread.schedule(query);
}

/*================================================================================*\
** Database Background Thread
\*================================================================================*/

DatabaseThread::DatabaseThread(LogWriter& logWriter)
: m_logWriter(logWriter),
  m_threadEnd(false)
{
    
}

DatabaseThread::~DatabaseThread()
{
    m_threadEnd = true;
    m_threadSemaphore.trigger();
    std::lock_guard<AtomicMutex> lock(m_threadLifetimeMutex);
}

void DatabaseThread::init()
{
    std::thread thread(threadProc, this);
    thread.detach();
}

void DatabaseThread::schedule(Query& query)
{
    m_inQueueMutex.lock();
    m_inQueue.emplace_back(std::move(query));
    m_inQueueMutex.unlock();
    m_threadSemaphore.trigger();
}

void DatabaseThread::threadProc(DatabaseThread* dbThread)
{
    std::lock_guard<AtomicMutex> lock(dbThread->m_threadLifetimeMutex);
    dbThread->threadLoop();
}

void DatabaseThread::swapAndPop(uint32_t i)
{
    if (m_threadProcessQueue.size() > 1)
    {
        // Swap
        m_threadProcessQueue[i] = std::move(m_threadProcessQueue.back());
    }
    
    // Pop
    m_threadProcessQueue.pop_back();
}

void DatabaseThread::threadLoop()
{
    m_logWriter.log(Log::Info, "Database Thread started");
    
    for (;;)
    {
        // Block until a query is scheduled or thread is flagged to end
        m_threadSemaphore.wait();
        
        for (;;)
        {
            // Check for newly scheduled queries
            m_inQueueMutex.lock();
            if (!m_inQueue.empty())
            {
                for (Query& query : m_inQueue)
                {
                    m_threadProcessQueue.emplace_back(std::move(query));
                }
                
                m_inQueue.clear();
            }
            m_inQueueMutex.unlock();
            
            // Attempt to execute any queries in our queue
            uint32_t i = 0;
            
            while (i < m_threadProcessQueue.size())
            {
                Query& query = m_threadProcessQueue[i];
                
                try
                {
                    if (query.execute())
                    {
                        if (query.hasCallback())
                        {
                            if (query.getState() == SQLITE_DONE)
                                query.setLastInsertId();
                            
                            m_outQueueMutex.lock();
                            m_outQueue.emplace_back(std::move(query));
                            m_outQueueMutex.unlock();
                        }
                        else
                        {
                            // Swap and pop alone won't destruct these properly
                            query.~Query();
                        }
                        
                        swapAndPop(i);
                        continue;
                    }
                    
                    i++;
                }
                catch (std::exception& e)
                {
                    m_logWriter.log(Log::Error, "[DatabaseThread] %s", e.what());
                    swapAndPop(i);
                }
            }
            
            if (m_threadProcessQueue.empty())
                break;
            
            Clock::sleepMilliseconds(2);
        }
        
        if (m_threadEnd)
            return;
    }
}

void DatabaseThread::executeQueryCallbacks()
{
    // We can only look at the out queue when we own its mutex.
    // However, we want to hold on to it for the shortest periods that we can,
    // particularly when a query's callback is running and we don't need it
    m_outQueueMutex.lock();
    
    while (!m_outQueue.empty())
    {
        // Scope to execute query's destructor before we re-lock
        {
            Query query(std::move(m_outQueue.back()));
            m_outQueue.pop_back();
            
            // Unlock the mutex while we execute the callback
            m_outQueueMutex.unlock();
            
            query.callback();
            // Destructor called here
        }
        
        // Re-lock the mutex before we do the empty check
        m_outQueueMutex.lock();
    }
    
    m_outQueueMutex.unlock();
}

/*================================================================================*\
** Query methods
\*================================================================================*/

Database::Query::Query()
: m_stmt(nullptr),
  m_database(nullptr),
  m_state(State::NotYetRun),
  m_userInt(0),
  m_queryNum(0),
  m_timestamp(0)
{
    
}

Database::Query::Query(Database::Query&& o)
: m_stmt(nullptr),
  m_database(nullptr),
  m_state(State::NotYetRun),
  m_userInt(0),
  m_queryNum(0),
  m_timestamp(0)
{
    *this = std::move(o);
}

Database::Query::~Query()
{
    sqlite3_finalize(m_stmt); // Safe if m_stmt is null
    m_stmt = nullptr;
}

Database::Query& Database::Query::operator=(Database::Query&& o)
{
    m_stmt          = o.m_stmt;
    m_database      = o.m_database;
    m_userInt       = o.m_userInt;
    m_lastInsertId  = o.m_lastInsertId;
    m_callback      = o.m_callback;
    m_queryNum      = o.m_queryNum;
    m_timestamp     = o.m_timestamp;
    
    o.m_stmt        = nullptr;
    o.m_state       = State::NotYetRun;
    
    return *this;
}

bool Database::Query::execute()
{
    int rc = sqlite3_step(m_stmt);
    
    switch (rc)
    {
    case SQLITE_BUSY:
        break;
    case SQLITE_ROW:
        setState(State::HasResults);
        goto ret_true;
    case SQLITE_DONE:
        setState(SQLITE_DONE);
        goto ret_true;
    default:
        throw Exception("[Database::Query::execute] %s", sqlite3_errstr(rc));
    }
    
    return false;
ret_true:
    // Logging is thread safe
    m_database->m_logWriter.log(Log::SQL, "Executed query %i against database '%s' in %lu microseconds",
        m_queryNum, m_database->m_dbPath.c_str(), Clock::microseconds() - m_timestamp);
    return true;
}

void Database::Query::executeSynchronus()
{
    for (;;)
    {
        if (execute())
            break;
    }
    
    if (getState() == SQLITE_DONE)
        setLastInsertId();
}

bool Database::Query::select()
{
    if (m_state == State::HasResults)
    {
        m_state = SQLITE_OK;
        return true;
    }
    
    int rc;
    
    do
    {
        rc = sqlite3_step(m_stmt);
    }
    while (rc == SQLITE_BUSY); // Shouldn't happen now
    
    switch (rc)
    {
    case SQLITE_ROW:
        return true;
    case SQLITE_DONE:
        sqlite3_reset(m_stmt);
        break;
    default:
        throw Exception("[Database::Query::select] %s", sqlite3_errstr(rc));
    }
    
    return false;
}

void Database::Query::setDb(Database* db)
{
    m_database  = db;
    m_queryNum  = ++db->m_queryNum;
    m_timestamp = Clock::microseconds();
}

void Database::Query::setLastInsertId()
{
    m_lastInsertId = sqlite3_last_insert_rowid(m_database->m_sqlite);
}

void Database::Query::bindInt(int col, int val)
{
    int rc = sqlite3_bind_int(m_stmt, col, val);
    
    if (rc != SQLITE_OK)
        throw Exception("[Database::Query::bindInt] %s", sqlite3_errstr(rc));
}

void Database::Query::bindInt64(int col, int64_t val)
{
    int rc = sqlite3_bind_int64(m_stmt, col, val);
    
    if (rc != SQLITE_OK)
        throw Exception("[Database::Query::bindInt64] %s", sqlite3_errstr(rc));
}

void Database::Query::bindDouble(int col, double val)
{
    int rc = sqlite3_bind_double(m_stmt, col, val);
    
    if (rc != SQLITE_OK)
        throw Exception("[Database::Query::bindDouble] %s", sqlite3_errstr(rc));
}

void Database::Query::bindString(int col, const std::string& val, bool copyString)
{
    bindString(col, val.c_str(), val.length(), copyString);
}

void Database::Query::bindString(int col, const char* str, uint32_t len, bool copyString)
{
    int rc = sqlite3_bind_text(m_stmt, col, str, len, copyString ? SQLITE_TRANSIENT : SQLITE_STATIC);
    
    if (rc != SQLITE_OK)
        throw Exception("[Database::Query::bindString] %s", sqlite3_errstr(rc));
}

void Database::Query::bindBlob(int col, const void* data, uint32_t len, bool copyBlob)
{
    int rc = sqlite3_bind_blob(m_stmt, col, data, len, copyBlob ? SQLITE_TRANSIENT : SQLITE_STATIC);
    
    if (rc != SQLITE_OK)
        throw Exception("[Database::Query::bindBlob] %s", sqlite3_errstr(rc));
}

int Database::Query::getInt(int col)
{
    return sqlite3_column_int(m_stmt, col - 1);
}

int64_t Database::Query::getInt64(int col)
{
    return sqlite3_column_int64(m_stmt, col - 1);
}

double Database::Query::getDouble(int col)
{
    return sqlite3_column_double(m_stmt, col - 1);
}

const char* Database::Query::getString(int col)
{
    return (const char*)sqlite3_column_text(m_stmt, col - 1);
}

const char* Database::Query::getString(int col, uint32_t& length)
{
    col--;
    length = sqlite3_column_bytes(m_stmt, col);
    return (const char*)sqlite3_column_text(m_stmt, col);
}

void Database::Query::getString(int col, std::string& str)
{
    col--;
    uint32_t length = sqlite3_column_bytes(m_stmt, col);
    str.assign((const char*)sqlite3_column_text(m_stmt, col), length);
}

const byte* Database::Query::getBlob(int col, uint32_t& length)
{
    col--;
    length = sqlite3_column_bytes(m_stmt, col);
    return (const byte*)sqlite3_column_blob(m_stmt, col);
}
