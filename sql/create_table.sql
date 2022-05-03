CREATE TABLE IF NOT EXISTS muser(
  uid INTEGER PRIMARY KEY AUTOINCREMENT,
  passwd VARCHAR(32) NOT NULL,
  uname VARCHAR(32) NOT NULL,
  status INTEGER DEFAULT 0,
  time TIMESTAMP DEFAULT NULL
);

CREATE TABLE IF NOT EXISTS mfriend(
  uid1 INTEGER PRIMARY KEY NOT NULL,
  uid2 INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS mgroup(
  gid INTEGER PRIMARY KEY AUTOINCREMENT,
  uid INTEGER NOT NULL,
  gname VARCHAR(32) NOT NULL
);

CREATE TABLE IF NOT EXISTS mgroupuser(
  gid INTEGER NOT NULL,
  uid INTEGER NOT NULL,
  PRIMARY KEY (gid, uid)
);

CREATE TABLE IF NOT EXISTS mmessage(
  uid INTEGER NOT NULL,
  type INTEGER NOT NULL,
  tid INTEGER NOT NULL,
  msg TEXT NOT NULL,
  time TIMESTAME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS mfile (
  uid INTEGER NOT NULL,
  tid INTEGER NOT NULL,
  filename TEXT NOT NULL,
  size INTEGER NOT NULL,
  time TIMESTAMP DEFAULT NULL,
  status INTEGER DEFAULT 0,
  curr INTEGER DEFAULT 0
);

-- 每次新建表时重置登陆状态
UPDATE
  muser
SET
  status = 0,
  time = NULL;