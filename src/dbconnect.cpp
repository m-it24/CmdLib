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

#include "dbconnect.h"

/**
 * @brief DBControl::DBControl
 *   Constructor of the class DBControl.
 */
DBConnect::DBConnect()
{
    return;
}

/**
 * @brief DBControl::~DBControl
 *   Destructor of the class DBControl.
 */
DBConnect::~DBConnect()
{
    return;
}

/**
 * @brief DBControl::open
 * @return
 */
DBConnect::byte DBConnect::open(string,ios::openmode) {
  #ifdef DEBUG
    cout << "ERROR! No implementation of open() available.\n";
  #endif
    return 240;
}

/**
 * @brief DBControl::read
 * @return
 */
list<string> DBConnect::read() {
    list<string> lst;
  #ifdef DEBUG
    cout << "ERROR! No implementation of read() available.\n";
  #endif
    return lst;
}

/**
 * @brief DBControl::write
 * @return
 */
DBConnect::byte DBConnect::write(list<string>,int) {
  #ifdef DEBUG
    cout << "ERROR! No implementation of write() available.\n";
  #endif
    return 240;
}

/**
 * @brief DBControl::close
 * @return
 */
DBConnect::byte DBConnect::close() {
  #ifdef DEBUG
    cout << "ERROR! No implementation of close() available.\n";
  #endif
    return 240;
}
