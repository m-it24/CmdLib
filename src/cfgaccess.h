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

#ifndef CFGACCESS_H
#define CFGACCESS_H

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include "main.h"

const char cfgtitle[] = "### CONFIGURATION COMMAND LIBRARY ###\n";

typedef unsigned char byte;

using namespace std;

class CfgAccess
{
    typedef unsigned char byte;

    // cfgstream is a logical device of type fstream connected
    // to a physical device (=file) via open
    fstream cfgstream;

    string filename;

    int lineEndCR;

    // structure of a data record
    struct record {
        string key;
        string value;
    };

    typedef list<record>::iterator iterRec;

    typedef enum { NO_RESULT, CONFIG_READ, CONFIG_WRITTEN, FILE_NOT_FOUND, FILE_IDENT_WRONG } IORESULT;

    list<record> records;  // contains the configurations from the configuration file

    IORESULT ioresult = NO_RESULT;

  public:
    CfgAccess();
    byte setConfig(string fn);    // sets the filename of the configuration file
    byte readConfig();            // opens the configuration file and reads the configuration
    byte writeConfig();           // writes the configuration to the configuration file
    string getValue(string key);  // returns the value of a key
    byte setValue(string key, string value);  // sets the value of a key
    byte removeKey(string key);   // removes a key from the configuration
};

#endif // CFGACCESS_H
