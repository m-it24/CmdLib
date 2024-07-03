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

#include "dbtext.h"

/**
 * @brief DBText::DBText
 *   Constructor of the class DBText.
 */
DBText::DBText()
{
    lineEndCR = 0;
    return;
}

/**
 * @brief DBText::open
 *   Opens the database for reading or writing.
 * @param filename
 * @param mode
 * @return opening result
 *   0 = database opened
 *   1 = database not found
 *   2 = database identifier not found
 *  10 = no valid mode for opening passed
 */
DBText::byte DBText::open(string filename,ios::openmode mode)
{
    byte result = 0;

    // open the database
    dbstream.open(filename,mode);

    // Database opened for reading?
    if (mode == (ios::in)) {

        // Database exists?
        if (dbstream) {
            char entry[256];
            //int  size;
            string dbtitle_str(dbtitle);
            string dbident_str;

            // check the database identifier on the first line
            dbstream.getline(entry,256);
            //size = dbstream.gcount();
            dbident_str+= entry;
            // Is the ending of the line terminated with carriage return?
            if (dbident_str.at(dbident_str.size()-1) == 13) {
                dbident_str = dbident_str.substr(0,dbident_str.size()-1);
                lineEndCR = 1;
              #ifdef DEBUG
                cout << "Windows type database file.\n";
              #endif
            }
            dbident_str+= '\n';

            // Database identifier not found?
            if (dbident_str != dbtitle_str) {
                dbstream.close();
                result = 2;
            }
        } else {
          #ifdef DEBUG
            cout << "ERROR! Can't open the file.\n";
          #endif
            result = 1;
        }
    // Database opened for writing?
    } else if (mode == (ios_base::out|ios_base::trunc)) {

        // Does the database open for writing?
        if (dbstream) {
            // write the database identifier to the file
            dbstream.write(dbtitle,sizeof(dbtitle)-1);
        } else {
          #ifdef DEBUG
            cout << "ERROR! Can't create the file.\n";
          #endif
            result = 1;
        }
    } else {
      #ifdef DEBUG
        cout << "ERROR! No valid mode for opening passed.\n";
      #endif
        result = 10;
    }

    cout.flush();

    return result;
}

/**
 * @brief DBText::read
 *   Reads a data record from the database.
 * @return a data record
 */
list<string> DBText::read()
{
    char entry[256];
    int  size;
    int  step = 1;
    int  dec_place;
    int  lines_note;

    string command;
    string category;

    list<string> record;

    do {
        dbstream.getline(entry,256);
        size = dbstream.gcount();
        // Windows file?
        if (lineEndCR == 1) {
            size--;
            entry[size-1] = 0;
        }
        if (size > 0) {
            switch (step) {
                case 1:
                    lines_note = 0;
                    dec_place  = 3;

                    for (int pos = 0; pos < size; pos++) {
                        switch (step) {
                            case 1:
                                // read the category
                                if (entry[pos] != ':') {
                                    category+= entry[pos];
                                } else {
                                    step++;
                                }
                                break;
                            case 2:
                                // read the number of lines of the notes
                                if (entry[pos] != ':') {
                                    switch (dec_place) {
                                        case 3: { lines_note+= (entry[pos] - 0x30) * 100; dec_place--; } break;
                                        case 2: { lines_note+= (entry[pos] - 0x30) * 10;  dec_place--; } break;
                                        case 1: { lines_note+= (entry[pos] - 0x30); } break;
                                    }
                                } else {
                                    step++;
                                }
                                break;
                            case 3:
                                // read the command
                                if (entry[pos] != 0x00) {
                                    command+= entry[pos];
                                } else {
                                    // place command and category in the data record list
                                    record.push_back(command);
                                    record.push_back(category);

                                    // Are there notes for the command?
                                    if (lines_note > 0) {
                                        step++;
                                    } else {
                                        // there are no notes => finish the reading
                                        step = 5;
                                    }
                                }
                                break;
                        }
                    }
                    break;
                case 4:
                    // add the notes to the data record list
                    record.push_back(*(new string(entry)));
                    lines_note--;
                    // All notes added? => finish the reading
                    if (lines_note <= 0) {
                        step = 5;
                    }
                    break;
            }
        }
    } while (step < 5 && size > 0);

    return record;
}

/**
 * @brief DBText::write
 *   Writes a data record to the database.
 * @return writing result
 */
DBText::byte DBText::write(list<string> record, int size_notes)
{
    byte result = 0;
    string entry;

    char place_h;
    char place_t;
    char place_u;

    place_h = (size_notes / 100) + 0x30; size_notes%= 100;
    place_t = (size_notes / 10) + 0x30;  size_notes%= 10;
    place_u = (size_notes) + 0x30;

    iterStr ptr_rec = record.begin();

    string command = *ptr_rec;
    ptr_rec++;
    string category = *ptr_rec;
    ptr_rec++;

    entry = category;
    entry+= ':';
    entry+= place_h;
    entry+= place_t;
    entry+= place_u;
    entry+= ':';
    entry+= command;
    entry+= '\n';

    dbstream.write(entry.c_str(),entry.size());

    for (iterStr ptr_notes = ptr_rec; ptr_notes != record.end(); ptr_notes++) {
        entry = *ptr_notes;
        entry+= '\n';
        dbstream.write(entry.c_str(),entry.size());
    }

    return result;
}

/**
 * @brief DBText::close
 *   Closes the database.
 * @return closing result
 */
DBText::byte DBText::close()
{
    byte result = 0;

    dbstream.close();

    return result;
}
