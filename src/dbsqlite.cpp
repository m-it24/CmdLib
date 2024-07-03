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

#include "dbsqlite.h"

/**
 * @brief DBSQLite::DBSQLite
 *   Constructor of the class DBSQLite.
 */
DBSQLite::DBSQLite()
{
    return;
}

/**
 * @brief DBSQLite::open
 *   Opens the database for reading or writing.
 * @param filename
 * @param mode
 * @return opening result
 *   0 = database opened
 *   1 = database not found
 *   2 = database isn't a SQL database
 *   3 = failed to open the database
 *  10 = no valid mode for opening passed
 */
DBSQLite::byte DBSQLite::open(string filename,ios::openmode mode)
{
    byte result  = 0;
    bool dbFound = false;

    dbConnection = QSqlDatabase::addDatabase("QSQLITE");

    dbConnection.setDatabaseName(QString(filename.c_str()));

    // check if the database exists
    fstream dbstream;
    dbstream.open(filename,ios::in);

    // Database exists?
    if (dbstream) {
        dbFound = true;
        dbstream.close();
    }

    // Database opened for reading?
    if (mode == ios::in) {

        // Database exists?
        if (dbFound) {
            if (dbConnection.open()) {

                QStringList tables = dbConnection.tables(QSql::Tables);

                // SQL database?
                if (tables.empty()) {
                  #ifdef DEBUG
                    cout << "ERROR! Database not a SQL database.\n";
                  #endif
                    result = 2;
                }
            } else {
              #ifdef DEBUG
                cout << "ERROR! Can't open the database.\n";
              #endif
                result = 3;
            }
        } else {
          #ifdef DEBUG
            cout << "ERROR! Database doesn't exist.\n";
          #endif
            result = 1;
        }
    // Database opened for writing?
    } else if (mode == (ios_base::out|ios_base::trunc)) {

        if (dbConnection.open()) {

            // Database exists?
            if (dbFound) {

                QStringList tables = dbConnection.tables(QSql::Tables);

                // SQL database?
                if (tables.empty()) {
                  #ifdef DEBUG
                    cout << "ERROR! Database not a SQL database.\n";
                  #endif
                    result = 2;
                } else {
                    clrRecordsUnused = true;
                }
            } else {
                // create the table for the commands
                QSqlQuery query = dbConnection.exec("CREATE TABLE commands ("
                                                    "cmd_id INTEGER NOT NULL PRIMARY KEY,"
                                                    "command VARCHAR(255),"
                                                    "category VARCHAR(64),"
                                                    "notes TEXT)");
                cout << getLastError();
            }
        } else {
          #ifdef DEBUG
            cout << "ERROR! Can't open the database.\n";
          #endif
            result = 3;
        }
    } else {
      #ifdef DEBUG
        cout << "ERROR! No valid mode for opening passed.\n";
      #endif
        result = 10;
    }

    // Error occured?
    if (result != 0) {
        // close the database connection
        dbConnection.close();
    }

    cout.flush();

    return result;
}

/**
 * @brief DBSQLite::read
 *   Reads a data record from the database.
 * @return a data record
 */
list<string> DBSQLite::read()
{
    volatile bool resQuery;

    list<string> record;

    if (!allRecordsRead) {
        query = new QSqlQuery(dbConnection);

        resQuery = query->prepare("SELECT command,category,notes FROM commands");

        if (!query->exec()) {
            cout << getLastError();
        } else {
            allRecordsRead = true;
        }
    }

    // Is there still a command in the query?
    if (query->next()) {
        QString command  = query->value(0).toString();
        QString category = query->value(1).toString();
        QByteArray notes = query->value(2).toByteArray();

        record.push_back(command.toStdString());
        record.push_back(category.toStdString());

        QByteArray::iterator ptr_nts = notes.begin();

        string note;

        while (ptr_nts != notes.end()) {
            if (*ptr_nts == '\0') {
                record.push_back(note);
                note.clear();
            } else {
                note+= *ptr_nts;
            }
            ptr_nts++;
        }
    }

    cout.flush();

    return record;
}

/**
 * @brief DBSQLite::write
 *   Writes a data record to the database.
 * @param record
 * @param size_notes
 * @return writing result
 */
DBSQLite::byte DBSQLite::write(list<string> record, int size_notes)
{
    byte result = 0;
    volatile bool resQuery;

    iterStr ptr_rec = record.begin();

    query = new QSqlQuery(dbConnection);

    string command = *ptr_rec;
    ptr_rec++;
    string category = *ptr_rec;
    ptr_rec++;

    resQuery = query->prepare("SELECT command FROM commands WHERE command = '"+QString(command.c_str())+"'");

    if (query->exec()) {

        resQuery = query->next();

        query->finish();

        if (resQuery) {
            // Command already exists in the database and has to modified.
          #ifdef DEBUG
            cout << "Command already exists in the database.\n";
          #endif
        } else {
            // Command doesn't exist in the database and has to inserted.
          #ifdef DEBUG
            cout << "Command doesn't exist in the database.\n";
          #endif

            resQuery = query->prepare("INSERT INTO commands(command,category,notes) VALUES (?,?,?)");

            query->addBindValue(QVariant(QString(command.c_str())));
            query->addBindValue(QString(category.c_str()));

            QByteArray notes;

            while (size_notes > 0) {
                string note = *ptr_rec;
                notes.append(note.c_str(),note.length()+1);
              //notes+= QString(*ptr_rec->c_str());
              //notes.append(QString(*ptr_rec->c_str()));
                ptr_rec++;
                size_notes--;
            }

            query->addBindValue(QVariant(notes));
            query->exec();
            query->finish();
        }

        resQuery = query->prepare("SELECT cmd_id FROM commands WHERE command = '"+QString(command.c_str())+"'");
        resQuery = query->exec();
        resQuery = query->next();

        cmdIDs.push_back(query->value(0).toInt());
    } else {
        cout << getLastError();
        result = 1;
    }

    cout.flush();

    delete query;
    query = nullptr;

    return result;
}

/**
 * @brief DBSQLite::close
 *   Closes the database.
 * @return closing result
 */
DBSQLite::byte DBSQLite::close()
{
    byte result = 0;
    volatile bool resQuery;

    if (clrRecordsUnused) {
        query = new QSqlQuery(dbConnection);

        resQuery = query->prepare("SELECT cmd_id FROM commands");
        resQuery = query->exec();

        list<int> cmdIDsDB;

        while (query->next()) {
            cmdIDsDB.push_back(query->value(0).toInt());
        }

        query->finish();

        list<int>::iterator iterIDsDB = cmdIDsDB.begin();
        list<int>::iterator iterIDsCmds;

        bool idFound;

        while (iterIDsDB != cmdIDsDB.end()) {
            int cmdID = *iterIDsDB;
            iterIDsDB++;
            iterIDsCmds = cmdIDs.begin();
            idFound = false;
            while (iterIDsCmds != cmdIDs.end() && !idFound) {
                if (cmdID == *iterIDsCmds) idFound = true;
                iterIDsCmds++;
            }
            if (!idFound) {
                resQuery = query->prepare("DELETE FROM commands WHERE cmd_id = '"+QString::number(cmdID)+"'");
                resQuery = query->exec();
            }
        }
    }

    allRecordsRead   = false;
    clrRecordsUnused = false;

    dbConnection.close();

    if (query != nullptr) delete query;

    return result;
}

/**
 * @brief DBSQLite::getLastError
 *   Determines the last occured database error.
 * @return last occured error
 */
string DBSQLite::getLastError()
{
    string errorStr;
    QSqlError lastError = dbConnection.lastError();
    QSqlError::ErrorType errorType = lastError.type();

    switch (errorType) {
        case QSqlError::NoError:
            errorStr = "No error.\n";
            break;
        case QSqlError::ConnectionError:
            errorStr = "ERROR(1)! Connection error.\n";
            break;
        case QSqlError::StatementError:
            errorStr = "ERROR(2)! Statement error.\n";
            break;
        case QSqlError::TransactionError:
            errorStr = "ERROR(3)! Transaction error.\n";
            break;
        default:
        case QSqlError::UnknownError:
            errorStr = "ERROR(4)! Unknown error.\n";
            break;
    }

    return errorStr;
}
