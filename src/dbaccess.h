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

#ifndef DBACCESS_H
#define DBACCESS_H

#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include "dbconnect.h"
#include "dbtext.h"
#include "dbsqlite.h"

using namespace std;

class DBAccess
{
    typedef unsigned char byte;

    // structure of a data record
    struct record {
        string command;
        string category;
        list<string> notes;
    };

    typedef list<record>::iterator iterRec;
    typedef list<string>::iterator iterStr;

    list<record> records;  // contains the data records from the database

    DBConnect *dbConnect;

  public:
    DBAccess();
    byte openRead(string fn);          // opens the database for reading data records
    byte openWrite(string fn);         // opens the database for writing data records
    byte read();                       // reads all commands from the database to a data records list
    byte write();                      // writes the data records list to the database
    byte close();                      // closes the database
    string getSuffix(string fn);       // returns the suffix of a filename
    byte clear();                      // removes the data records from the data records list
    list<string> catRead();            // reads all categories the database contains
    list<string> cmdRead(string cat);  // reads all commands of a category
    list<string> ntsRead(string cat, string cmd);  // reads the notes of a command
    byte cmdAdd(list<string> cmd);     // adds a new command with category and notes to the data records list
    byte cmdModify(list<string> cmd);  // modifies an edited command including the notes
    byte cmdDelete(list<string> cmd);  // deletes a command from the database
    int  recFind(string cat, string cmd);  // finds a command in the database
    byte cmdsSort();                   // sorts the database
};

#endif // DBACCESS_H
