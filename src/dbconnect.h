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

#ifndef DBCONNECT_H
#define DBCONNECT_H

#include <ios>
#include <iostream>
#include <list>
#include <string>
#include "main.h"

using namespace std;

class DBConnect
{
    typedef unsigned char byte;

  public:
    DBConnect();
    virtual ~DBConnect();

    virtual byte open(string,ios::openmode);  // opens the database for reading or writing
    virtual list<string> read();              // reads a data record from the database
    virtual byte write(list<string>,int);     // writes a data record to the database
    virtual byte close();                     // closes the database
};

#endif // DBCONNECT_H
