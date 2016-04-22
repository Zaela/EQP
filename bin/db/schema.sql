
-- Enable WAL mode
PRAGMA journal_mode = WAL;

CREATE TABLE test (
    x INT
);

INSERT INTO test VALUES
    (random()), (random()), (random()), (random()), (random()),
    (random()), (random()), (random()), (random()), (random()),
    (random()), (random()), (random()), (random()), (random());
    
-- Only used by the minimal, localhost-only login server
CREATE TABLE local_login (
    username    TEXT PRIMARY KEY,
    password    BLOB,
    salt        BLOB
);
