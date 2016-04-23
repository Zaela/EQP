
-- Enable WAL mode
PRAGMA journal_mode = WAL;
    
-- Only used by the minimal, localhost-only login server
CREATE TABLE local_login (
    username    TEXT PRIMARY KEY,
    password    BLOB,
    salt        BLOB
);

-- Most values are sent by the EQEmu login server when a client is about to log in
CREATE TABLE account (
    login_server_id             INT PRIMARY KEY,
    name                        TEXT,
    most_recent_character_name  TEXT,
    status                      INT DEFAULT 0,
    gm_speed                    BOOLEAN DEFAULT 0,
    gm_hide                     BOOLEAN DEFAULT 0,
    shared_platinum             INT DEFAULT 0,
    suspended_until             INT DEFAULT 0,
    creation_time               DATE DEFAULT 0
);

CREATE TRIGGER trigger_account_creation_time AFTER INSERT ON account
BEGIN
    UPDATE account SET creation_time = datetime('now') WHERE login_server_id = new.login_server_id;
END;
