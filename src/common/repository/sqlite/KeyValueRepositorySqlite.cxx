//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2020 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "KeyValueRepositorySqlite.hxx"
#include "Logger.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KeyValueRepositorySqlite::KeyValueRepositorySqlite(
  SqliteDatabase& db, const string& tableName)
  : myTableName{tableName},
    myDb{db}
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SqliteStatement& KeyValueRepositorySqlite::stmtInsert(const string& key, const string& value)
{
  myStmtInsert->reset();

  return (*myStmtInsert)
    .bind(1, key.c_str())
    .bind(2, value.c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SqliteStatement& KeyValueRepositorySqlite::stmtSelect()
{
  myStmtInsert->reset();

  return *myStmtSelect;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SqliteStatement& KeyValueRepositorySqlite::stmtDelete(const string& key)
{
  myStmtDelete->reset();

  return (*myStmtDelete)
    .bind(1, key.c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SqliteStatement& KeyValueRepositorySqlite::stmtCount(const string& key)
{
  myStmtCount->reset();

  return (*myStmtCount)
    .bind(1, key.c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SqliteStatement& KeyValueRepositorySqlite::stmtSelectOne(const string& key)
{
  myStmtSelectOne->reset();

  return (*myStmtSelectOne)
    .bind(1, key.c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SqliteDatabase& KeyValueRepositorySqlite::database()
{
  return myDb;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KeyValueRepositorySqlite::initialize()
{
  myDb.exec(
    "CREATE TABLE IF NOT EXISTS `" + myTableName + "` (`key` TEXT PRIMARY KEY, `value` TEXT) WITHOUT ROWID"
  );

  myStmtInsert = make_unique<SqliteStatement>(myDb, "INSERT OR REPLACE INTO `" + myTableName + "` VALUES (?, ?)");
  myStmtSelect = make_unique<SqliteStatement>(myDb, "SELECT `key`, `value` FROM `" + myTableName + "`");
  myStmtDelete = make_unique<SqliteStatement>(myDb, "DELETE FROM `" + myTableName + "` WHERE `key` = ?");
  myStmtSelectOne = make_unique<SqliteStatement>(myDb, "SELECT `value` FROM `" + myTableName + "` WHERE `key` = ?");
  myStmtCount = make_unique<SqliteStatement>(myDb, "SELECT COUNT(`key`) FROM `" + myTableName + "` WHERE `key` = ?");
}
