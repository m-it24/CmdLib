/*****************************************************************************
    Copyright (C) 2024 Rainer Otto <ro2611@m-it-rheinruhr.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef DBSQLITE_H
#define DBSQLITE_H

#include <fstream>
#include <string>
#include <QByteArray>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtSql>
#include "dbconnect.h"
#include "main.h"

class DBSQLite : public DBConnect
{
    typedef unsigned char byte;

    typedef list<string>::iterator iterStr;

    list<int> cmdIDs;

    bool allRecordsRead   = false;
    bool clrRecordsUnused = false;

    QSqlDatabase dbConnection;
    QSqlQuery    *query = nullptr;

  public:
    DBSQLite();
    byte open(string filename, ios::openmode mode);   // opens the database for reading or writing
    list<string> read();                              // reads a data record from the database
    byte write(list<string> record, int size_notes);  // writes a data record to the database
    byte close();                                     // closes the database
    string getLastError();                            // determines the last occured database error
};

#endif // DBSQLITE_H
