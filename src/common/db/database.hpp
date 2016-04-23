
#ifndef _EQP_DATABASE_HPP_
#define _EQP_DATABASE_HPP_

#include "define.hpp"
#include "atomic_mutex.hpp"
#include "semaphore.hpp"
#include "file.hpp"
#include "exception.hpp"
#include "clock.hpp"
#include "log_writer.hpp"
#include <sqlite3.h>
#include <functional>
#include <string>
#include <thread>
#include <chrono>

#define EQP_SQLITE_MAIN_DATABASE_PATH   "db/eqp.db"
#define EQP_SQLITE_MAIN_SCHEMA_PATH     "db/schema.sql"

class DatabaseThread;

// Generic SQLite Database class, representing one database file on disk.
//
// Queries are queued in the main thread, then executed in a dedicated background thread;
// when a query completes, an associated callback will be invoked in the main thread on 
// the next cycle to read SELECT data or otherwise respond to query success or failure.
//
// Multiple database files may be opened, each sharing the same single background thread.
// The background thread does not need to know which query is being executed against
// which database file.
//
// SQLite's write-ahead logging (WAL) mode is used on the main, relatively frequently 
// written-to database files to reduce lock contention.
//
// One-off databases (say, one created and used by a single boss NPC script) may
// use the default 'rollback journal' mode or WAL mode; the background thread can
// handle either.
class Database
{
public:
    class Query
    {
    public:
        typedef std::function<void(Query&)> Callback;
    
    private:
        sqlite3_stmt*   m_stmt;
        Database*       m_database;
        union
        {
            int         m_state;
            int64_t     m_lastInsertId;
        };
        union
        {
            void*       m_userdata;
            int64_t     m_userInt;
        };
        Callback        m_callback;
        uint32_t        m_queryNum;
        uint64_t        m_timestamp;
    
    private:
        enum State : int
        {
            HasResults  = -1,
            NotYetRun   = -2
        };
    
    private:
        friend class DatabaseThread;
        friend class Database;
        void setStmt(sqlite3_stmt* stmt) { m_stmt = stmt; }
        void setDb(Database* db);
        void setState(int state) { m_state = state; }
        void setLastInsertId();
        void callback() { if (m_callback) m_callback(*this); }
        bool execute();
        
        bool hasCallback() { return m_callback ? true : false; }
        
        int getState() const { return m_state; }

    public:
        Query();
        Query(Query&& o);
        ~Query();
    
        Query& operator=(Query&& o);
    
        void executeSynchronus();
    
        void* userdata() const { return m_userdata; }
        int64_t userInt() const { return m_userInt; }
        
        void setUserdata(void* ud) { m_userdata = ud; }
        void setuserInt(int64_t val) { m_userInt = val; }
        void setCallback(Callback callback) { m_callback = callback; }
    
        bool select();

        void bindInt(int col, int val);
        void bindInt64(int col, int64_t val);
        void bindDouble(int col, double val);
        void bindString(int col, const std::string& val, bool copyString = true);
        void bindString(int col, const char* str, uint32_t len, bool copyString = true);
        void bindBlob(int col, const void* data, uint32_t len, bool copyBlob = true);

        int         getInt(int col);
        int64_t     getInt64(int col);
        double      getDouble(int col);
        const char* getString(int col);
        const char* getString(int col, uint32_t& length);
        void        getString(int col, std::string& str);
        const byte* getBlob(int col, uint32_t& length);
        
        int64_t getLastInsertId() const { return m_lastInsertId; }
    };
    
private:
    friend class Query;
    sqlite3*        m_sqlite;
    DatabaseThread& m_thread;
    LogWriter&      m_logWriter;
    std::string     m_dbPath;
    uint32_t        m_queryNum;

private:
    void createFromSchema(const char* dbPath, const char* schemaPath);

public:
    Database(DatabaseThread& thread, LogWriter& logWriter);
    ~Database();

    void init(const char* dbPath, const char* schemaPath);

    void prepare(Query& query, const std::string& sql, Query::Callback callback = Query::Callback());
    void beginTransaction();
    void commitTransaction();

    // exceptionFormat should have exactly one format specifier: a "%s" for the error message from SQLite
    void exec(const std::string& sql, const char* exceptionFormat);

    void schedule(Query& query);

    // For queries with no parameters
    void prepareAndSchedule(const std::string& sql, Query::Callback callback = Query::Callback());
};

typedef Database::Query Query;

class DatabaseThread
{
private:
    LogWriter&          m_logWriter;

    AtomicMutex         m_inQueueMutex;
    std::vector<Query>  m_inQueue;
    AtomicMutex         m_outQueueMutex;
    std::vector<Query>  m_outQueue;

    std::atomic_bool    m_threadEnd;
    Semaphore           m_threadSemaphore;
    AtomicMutex         m_threadLifetimeMutex;
    std::vector<Query>  m_threadProcessQueue;

private:
    void threadLoop();
    void swapAndPop(uint32_t i);
    static void threadProc(DatabaseThread* dbThread);
    
public:
    DatabaseThread(LogWriter& logWriter);
    ~DatabaseThread();

    void init();

    void schedule(Query& query);
    void executeQueryCallbacks();
};

#endif//_EQP_DATABASE_HPP_
