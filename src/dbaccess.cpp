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

#include "dbaccess.h"

/**
 * @brief DBAccess::DBAccess
 *   Constructor of the class DBAccess.
 */
DBAccess::DBAccess()
{
    return;
}

/**
 * @brief DBAccess::openRead
 *   Opens the database for reading data records.
 * @param fn  filename
 * @return opening result
 */
DBAccess::byte DBAccess::openRead(string fn)
{
    string suffix = getSuffix(fn);

    // SQLite database?
    if (suffix != "sqlite") {
        dbConnect = new DBText;
    } else {
        dbConnect = new DBSQLite;
    }

    // in=open the file for reading
    return dbConnect->open(fn/*"cmdlib.db"*/,/*ios_base::out|*/ios/*_base*/::in);
}

/**
 * @brief DBAccess::openWrite
 *   Opens the database for writing data records.
 * @param fn  filename
 * @return opening result
 */
DBAccess::byte DBAccess::openWrite(string fn)
{
    string suffix = getSuffix(fn);

    // SQLite database?
    if (suffix != "sqlite") {
        dbConnect = new DBText;
    } else {
        dbConnect = new DBSQLite;
    }

    // out=open the file for output, trunc=delete an existing file before opening
    return dbConnect->open(fn,ios_base::out|ios_base::trunc);
}

/**
 * @brief DBAccess::read
 *   Reads all commands from the database to a data records list.
 * @return reading result
 */
DBAccess::byte DBAccess::read()
{
    byte result = 0;
    record *rec = nullptr;

    list<string> recCmd;

    // reset the data records list
    records.clear();

    do {
      recCmd = dbConnect->read();

      if (!recCmd.empty()) {

        iterStr ptr_rec = recCmd.begin();

        rec = new record;

        rec->command = *ptr_rec;
        ptr_rec++;
        rec->category = *ptr_rec;
        ptr_rec++;

        for (iterStr ptr_notes = ptr_rec; ptr_notes != recCmd.end(); ptr_notes++) {
            rec->notes.push_back(*ptr_notes);
        }

        records.push_back(*rec);

        delete rec;
      }

    } while (!recCmd.empty());

    return result;
}

/**
 * @brief DBAccess::write
 *   Writes the data records list to the database.
 * @return writing result
 */
DBAccess::byte DBAccess::write()
{
    byte   result = 0;
    record rec;

    list<string> record;

    list<string>::size_type size_notes;

    // sort the data records
    cmdsSort();

    for (iterRec ptr = records.begin(); ptr != records.end(); ptr++) {

        rec = *ptr;

        size_notes   = rec.notes.size();
        int size_val = (int)size_notes;

        record.clear();
        record.push_back(rec.command);
        record.push_back(rec.category);

        for (iterStr ptr_notes = rec.notes.begin(); ptr_notes != rec.notes.end(); ptr_notes++) {
            record.push_back(*ptr_notes);
        }

        result = dbConnect->write(record,size_val);

        if (result != 0) break;
    }

    return result;
}

/**
 * @brief DBAccess::close
 *   Closes the database.
 * @return closing result
 */
DBAccess::byte DBAccess::close()
{
    int result = dbConnect->close();

    delete dbConnect;

    return result;
}

/**
 * @brief getSuffix
 *   Returns the suffix of a filename.
 * @param fn  filename
 * @return suffix of the filename in lower-case letters
 */
string DBAccess::getSuffix(string fn)
{
    string suffix;
    string::iterator iterStr = fn.end();

    iterStr--;

    // determine the suffix of the filename
    while (*iterStr != '.' && iterStr != fn.begin()) {
        if (*iterStr >= 'A' && *iterStr <= 'Z') *iterStr+= 32;
        suffix = *iterStr + suffix;
        *iterStr--;
    }

    return suffix;
}

/**
 * @brief DBAccess::clear
 *   Removes the data records from the data records list.
 * @return removing result
 */
DBAccess::byte DBAccess::clear()
{
    byte result = 0;

    records.clear();  // clear all data records from the data records list

    return result;
}

/**
 * @brief DBAccess::catRead
 *   Reads all categories from the command list.
 * @return list with all categories
 */
list<string> DBAccess::catRead()
{
    list<string> catLst;

    for (iterRec ptrRec = records.begin(); ptrRec!= records.end(); ptrRec++) {

        string cat = (*ptrRec).category;

        // write categories only once to the category list
        iterStr ptrStr = find(catLst.begin(),catLst.end(),cat);

        if (ptrStr == catLst.end()) {
            catLst.push_back(cat);
        }
    }

    return catLst;
}

/**
 * @brief DBAccess::cmdRead
 *   Reads all commands of a category.
 * @param cat  category
 * @return list with all commands of a category
 */
list<string> DBAccess::cmdRead(string cat)
{
    list<string> cmdLst;

    for (iterRec ptrRec = records.begin(); ptrRec!= records.end(); ptrRec++) {

        if ((*ptrRec).category == cat) {
            cmdLst.push_back((*ptrRec).command);
        }
    }

    return cmdLst;
}

/**
 * @brief DBAccess::ntsRead
 *   Reads the notes of a command.
 * @param cat  category
 * @param cmd  command
 * @return notes of a command
 */
list<string> DBAccess::ntsRead(string cat, string cmd)
{
    list<string> notes;

    int numRec = recFind(cat,cmd);

    // Data record found?
    if (numRec >= 0) {

        iterRec iter = records.begin();

        for (int cntr = 0; cntr < numRec; cntr++) {
            iter++;
        }

        notes = iter->notes;
    }

    return notes;
}

/**
 * @brief DBAccess::cmdAdd
 *   Adds a new command with category and notes to the data records list.
 * @param cmd  list containing command, category and notes
 * @return adding result
 */
DBAccess::byte DBAccess::cmdAdd(list<string> cmd)
{
    byte result = 0;
    record rec;

    rec.category = cmd.front();
    cmd.pop_front();
    rec.command = cmd.front();
    cmd.pop_front();
    rec.notes = cmd;

    iterStr ptr_notes = rec.notes.end();
    ptr_notes--;
    if (ptr_notes->size() == 0) {
        rec.notes.pop_back();
    }

    records.push_back(rec);

    return result;
}

/**
 * @brief DBAccess::cmdModify
 *   Modifies an edited command including the notes.
 * @param cmd  list containing the modification
 * @return 0 = command modified
 *         1 = data record not found
 */
DBAccess::byte DBAccess::cmdModify(list<string> cmd)
{
    byte result = 0;

    string cat = cmd.front();
    cmd.pop_front();
    string cmdOld = cmd.front();
    cmd.pop_front();
    string cmdNew = cmd.front();
    cmd.pop_front();
    list<string>notes = cmd;

    iterStr ptrStr = notes.begin();

    if (*ptrStr == "") {
        notes.clear();
    }

    int numRec = recFind(cat,cmdOld);

    // Data record found?
    if (numRec >= 0) {

        iterRec ptrRec = records.begin();

        for (int cntr = 0; cntr < numRec; cntr++) {
            ptrRec++;
        }

        (*ptrRec).command = cmdNew;
        (*ptrRec).notes   = notes;

    } else {
        result = 1;
    }

    return result;
}

/**
 * @brief DBAccess::cmdDelete
 *   Deletes a command from the database.
 * @param cmd  command to delete
 * @return deleting result
 */
DBAccess::byte DBAccess::cmdDelete(list<string> cmd)
{
    byte result = 0;

    string cat = cmd.front();
    cmd.pop_front();
    string cmdDel = cmd.front();
    cmd.pop_front();

    int numRec = recFind(cat,cmdDel);

    // Data record found?
    if (numRec >= 0) {

        iterRec ptrRec = records.begin();

        for (int cntr = 0; cntr < numRec; cntr++) {
            ptrRec++;
        }

        records.erase(ptrRec);

    } else {
        result = 1;
    }

    return result;
}

/**
 * @brief DBAccess::find
 *   Finds a command in the database.
 * @param cat  category
 * @param cmd  command to find
 * @return >= 0 = record number
 *           -1 = record not found
 */
int DBAccess::recFind(string cat, string cmd)
{
    int     numRec = -1;
    iterRec ptrRec;

    for (ptrRec = records.begin(); ptrRec!= records.end(); ptrRec++) {

        numRec++;

        if (((*ptrRec).category == cat) &&
            ((*ptrRec).command == cmd)) {
            break;
        }
    }

    if (ptrRec == records.end()) { numRec = -1; }

    return numRec;
}

/**
 * @brief DBAccess::cmdsSort
 *   Sorts the database.
 * @return
 */
DBAccess::byte DBAccess::cmdsSort()
{
    byte   result = 0;
    record swapRec;

    iterRec iter1 = records.begin();
    iterRec iter2;

    // first sort the commands of the data records
    for (int idx1 = 0; idx1 < ((int)records.size()-1); idx1++) {
        iter2 = iter1;
        for (int idx2 = idx1+1; idx2 < (int)records.size(); idx2++) {
            iter2++;
            // Command of data record 2 < command of data record 1?
            if (iter2->command < iter1->command) {
              // swap the data records
                swapRec.command  = iter1->command;
                swapRec.category = iter1->category;
                swapRec.notes    = iter1->notes;
              //..
                iter1->command  = iter2->command;
                iter1->category = iter2->category;
                iter1->notes    = iter2->notes;
              //..
                iter2->command  = swapRec.command;
                iter2->category = swapRec.category;
                iter2->notes    = swapRec.notes;
            }
        }
        iter1++;
    }

    // then sort the categories of the data records
    iter1 = records.begin();

    for (int idx1 = 0; idx1 < ((int)records.size()-1); idx1++) {
        iter2 = iter1;
        for (int idx2 = idx1+1; idx2 < (int)records.size(); idx2++) {
            iter2++;
            // Category of data record 2 < category of data record 1?
            if (iter2->category < iter1->category) {
              // swap the data records
                swapRec.command  = iter1->command;
                swapRec.category = iter1->category;
                swapRec.notes    = iter1->notes;
              //..
                iter1->command  = iter2->command;
                iter1->category = iter2->category;
                iter1->notes    = iter2->notes;
              //..
                iter2->command  = swapRec.command;
                iter2->category = swapRec.category;
                iter2->notes    = swapRec.notes;
            }
        }
        iter1++;
    }

    return result;
}
