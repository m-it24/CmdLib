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

#include <iostream>
#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include "main.h"
#include "mainwindow.h"

/* Command Library */

const char title[] = "Command Library";

using namespace std;

/**
 * @brief createApplication
 *   If the application is started without parameters, the GUI version of the application
 *   is called. Passing parameters to the application at startup, the command line version
 *   of the application is called.
 * @param argc  number of parameters; &argc=reference to the variable passed to the function
 * @param argv  pointer to the array with the passed parameters
 * @return application object
 */
QCoreApplication* createApplication(int &argc, char *argv[])
{
    if (argc >= 2) {  // argc=1 => application is started without parameters => GUI version is called
                      // argc>1 => application is started with at least one parameter => command line version is called
        return new QCoreApplication(argc,argv);
    } else {
        // Every Qt application holds an object of class QApplication
        // for event processing and application management.
        return new QApplication(argc,argv);
    }
}

/**
 * @brief main
 * @param argc
 * @param argv
 * @return result of the application execution
 */
int main(int argc, char *argv[])
{
    int result = 0;

    // Defining an object of class QScopedPointer containing a pointer to a dynamic created object on the heap.
    QScopedPointer<QCoreApplication> app(createApplication(argc,argv));

    if (qobject_cast<QApplication *>(app.data())) {
      #ifdef DEBUG
        cout << "Start GUI version ..." << "\n";
      #endif
        MainWindow *mainWindow = new MainWindow();

        result = mainWindow->getDBAccessResult();
        // 0 = database opened
        // 1 = database not found
        // 2 = not a CLY/SQL database
        // 3 = error in opening SQL database

        // Database not opened?
        if (result != 0) {
            QMessageBox *msgBox = new QMessageBox();
            msgBox->setWindowTitle("Command Library");
            msgBox->setIcon(QMessageBox::Warning);
            msgBox->setText("Error! Not possible to open last used database.");
            msgBox->exec();
            delete msgBox;
        }

        // The following signal-slot connection closes the main window at first and
        // then runs possible queries in exit().
        //QObject::connect(app.data(),SIGNAL(aboutToQuit()),mainWindow,SLOT(exit()));
        mainWindow->setWindowTitle(title);
        mainWindow->show();

        // start the event processing
        result = app->exec();

        delete mainWindow;

    } else {
      #ifdef DEBUG
        cout << "Start non-GUI version ..." << "\n";
      #endif
    }

    return result;
}
