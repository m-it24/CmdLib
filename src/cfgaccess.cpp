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

#include "cfgaccess.h"

/**
 * @brief CfgAccess::CfgAccess
 *   Constructor of the class CfgAccess.
 */
CfgAccess::CfgAccess()
{
    lineEndCR = 0;
    return;
}

/**
 * @brief CfgAccess::setConfig
 *   Sets the filename of the configuration file.
 * @param fn
 * @return
 */
CfgAccess::byte CfgAccess::setConfig(string fn)
{
    filename = fn;
    return 0;
}

/**
 * @brief CfgAccess::readConfig
 *   Reads the configuration from the configuration file.
 * @return reading result
 *   0 = configuration read
 *   1 = could not open the configuration
 *   2 = configuration file identifier not found
 */
CfgAccess::byte CfgAccess::readConfig()
{
    byte result = 0;
    int  size;
    int  pos;
    char entry[256];
    string cfgtitle_str(cfgtitle);
    string cfgident_str;
    string keyValLine;
    record *rec = nullptr;

    ioresult = CONFIG_READ;

    // in=open the file for reading
    cfgstream.open(filename,ios::in);

    if (cfgstream) {
        // check the file identifier
        cfgstream.getline(entry,256);
        cfgident_str+= entry;
        // Does the line finish with a carriage return?
        if (cfgident_str.at(cfgident_str.size()-1) == 13) {
            cfgident_str = cfgident_str.substr(0,cfgident_str.size()-1);
            lineEndCR = 1;
          #ifdef DEBUG
            cout << "Windows type configuration file.\n";
          #endif
        }
        cfgident_str+= '\n';
        // File identifier found?
        if (cfgident_str == cfgtitle_str) {
            // read the configuration
            do {
                cfgstream.getline(entry,256);
                size = cfgstream.gcount();
                if (size > 0) {
                    keyValLine = entry;
                    pos = keyValLine.find('=',0);
                    rec = new record;
                    rec->key.assign(keyValLine,0,pos);
                    rec->value.assign(keyValLine,pos+1,size-(pos+1)-1-lineEndCR);  // -2 due to size contains the zero termination of entry
                    records.push_back(*rec);
                }
            } while (size > 0);

            cfgstream.close();
          #ifdef DEBUG
            cout << "Configuration read.\n";
          #endif
            ioresult = CONFIG_READ;
            result   = 0;
        } else {
            cfgstream.close();
          #ifdef DEBUG
            cout << "ERROR! Configuration file identifier doesn't exist or defective.\n";
          #endif
            ioresult = FILE_IDENT_WRONG;
            result   = 2;
        }
    } else {
      #ifdef DEBUG
        cout << "ERROR! Can't open the configuration file.\n";
      #endif
        ioresult = FILE_NOT_FOUND;
        result   = 1;
    }

    cout.flush();

    return result;
}

/**
 * @brief CfgAccess::writeConfig
 * @return
 */
CfgAccess::byte CfgAccess::writeConfig()
{
    byte result  = 0;
    iterRec iter = records.begin();
    string keyValLine;

    // out=open the file for output, trunc=delete an existing file before opening
    cfgstream.open(filename,ios_base::out|ios_base::trunc);

    if (cfgstream) {
        // write the file identifier to the configuration file
        keyValLine = cfgtitle;
        cfgstream.write(keyValLine.c_str(),keyValLine.size());

        while (iter!= records.end()) {
            keyValLine = iter->key;
            keyValLine+= '=';
            keyValLine+= iter->value;
            keyValLine+= '\n';
            cfgstream.write(keyValLine.c_str(),keyValLine.size());
            iter++;
        }
        cfgstream.close();
      #ifdef DEBUG
        cout << "Configuration written.\n";
      #endif
    } else {
      #ifdef DEBUG
        cout << "ERROR! Can't create the configuration file.\n";
      #endif
        result = 1;
    }

    cout.flush();

    return result;
}

/**
 * @brief CfgAccess::getValue
 * @param key
 * @return value assigned to the key
 */
string CfgAccess::getValue(string key)
{
    string value = "";
    iterRec iter = records.begin();

    while (iter!= records.end()) {
        if (iter->key == key) {
            value = iter->value;
            break;
        }
        iter++;
    }

    return value;
}

/**
 * @brief CfgAccess::setValue
 * @param key
 * @param value
 * @return 0 = value assigned to an existing key
 *         1 = new key added
 */
CfgAccess::byte CfgAccess::setValue(string key, string value)
{
    byte result  = 0;
    iterRec iter = records.begin();

    while (iter != records.end()) {
        if (iter->key == key) {
            iter->value = value;
            break;
        }
        iter++;
    }

    if (iter == records.end()) {
        record *rec = new record;
        rec->key = key;
        rec->value = value;
        records.push_back(*rec);
        result = 1;
    }

    return result;
}

/**
 * @brief removeKey
 * @param key
 * @return 0 = key removed from the configuration
 *         1 = key not found in the configuration
 */
CfgAccess::byte CfgAccess::removeKey(string key)
{
    byte result = 1;
    iterRec iter = records.begin();

    while (iter != records.end()) {
        if (iter->key == key) {
            records.erase(iter);
            result = 0;
            break;
        }
        iter++;
    }

    return result;
}
