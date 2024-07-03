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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <QComboBox>
#include <QDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QOperatingSystemVersion>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include <QWidget>
#include "adddialog.h"
#include "cfgaccess.h"
#include "dbaccess.h"
#include "introwindow.h"
#include "main.h"
#include "settingsdialog.h"
#include "terminalwindow.h"

// using namespace std no need of std::-notation is needed in the source code file
using namespace std;

/*QT_BEGIN_NAMESPACE
class QTextEdit;
class QLineEdit;
QT_END_NAMESPACE*/

/**
 * @brief MainWindow
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

    #define DBACCESSBUTTONSWIDTH  120

    /**
     * @brief CommandNotesWindow
     */
    class CommandNotesWindow : public QTextEdit
    {
        int width;
        int height;

        QSize sizeHint(void) const override;

      public:
        CommandNotesWindow(int w, int h);
    };

    enum OSTYPES { LINUX, WINDOWS, NONE } osType = NONE;

    QWidget     *widgetCentral = nullptr;
    QWidget     *widgetDBAccessButtons;
    QGridLayout *gridCentralWidget;
    QGridLayout *gridDBAccessButtons;
    QDockWidget *docWidgetLeft;
    QDockWidget *dockWidgetRight;
    QLabel      *labelTerminal;
    QLabel      *labelStatusBarLeft;
    QLabel      *labelStatusBarRight;
    QTextEdit   *textEditTerminal;
    QTextEdit   *textEditCommandNotes;
    QLineEdit   *lineEditLastCommand;
    QPushButton *buttonClr;
    QPushButton *buttonAdd;
    QPushButton *buttonMod;
    QPushButton *buttonDel;
    QMenuBar    *menuBarMain;
    QMenu       *menuDB;
    QAction     *menuDBEntry1;
    QAction     *menuDBEntry2;
    QAction     *menuDBEntry3;
    QAction     *menuDBEntry4;
    QAction     *menuDBEntry5;
    QAction     *menuDBEntry6;
    QMenu       *menuDBRecent;
    QMenu       *menuEdit;
    QAction     *menuEditEntry;
    QMenu       *menuView;
    QMenu       *menuHelp;
    QAction     *menuHelpEntry1;
    QAction     *menuHelpEntry2;
    QToolBar    *toolBar;
    QStatusBar  *statusBar;
    QComboBox   *combCategories;
    QComboBox   *combCommands;

    QString recentDB;

    CfgAccess cfgAccess;
    DBAccess  dbAccess;

    IntroWindow *introductionWindow;

    int dbAccessResult;

    int currCatNum;   // choosen category number
    int currCmdNum;   // choosen command number

    string currCat;   // choosen category
    string currCmd;   // choosen command
  //string lastCmd;   // last executed command
    string dbLast;    // last used database
    string dbName;    // database name without path
    string dbPath;    // path to the database
    string cfgFile;   // name of the configuration file
    string language;  // language used by the application

    list<string> catList;  // contains all available categories
    list<string> cmdList;  // contains the commands of the choosen category

    list<string> recentDBs;  // contains the recent used databases

    string recentLabels[5] = { "RECENT1", "RECENT2", "RECENT3", "RECENT4", "RECENT5" };

    typedef list<string>::iterator iterStr;

    typedef enum { DB_NO_STATE, DB_NEW, DB_NEW_EDITED, DB_LOADED, DB_MODIFIED } DBSTATE;

    DBSTATE dbState = DB_NO_STATE;

    int winPosX;
    int winPosY;
    int winWidth;
    int winHeight;
    int notesWidth;

    int internVar;

  private:
  //QSize sizeHint(void) const override;

    QLabel    *createTerminalLabel(void);
    QTextEdit *createTerminalWindow(void);
    QLineEdit *createLastCommandWindow(void);
    QTextEdit *createCommandNotesWindow(void);

    QString getDatabaseFile(QFileDialog::AcceptMode acceptMode, QFileDialog::FileMode fileMode);

    void setCategories(void);
    void getDBName(void);
    void setDBName(bool modified);
    void resetDB(void);
    void addRecentDB(string dbFile);
    void updateRecentDBs(void);
    void displayHintSQLite(void);
    void translateMainWindow();
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

  private slots:
    void setCommands(int);
    void setCommandSelected(int);
    void setCommandEntered(QString *);
  //..
    void buttonClearPressed();
    void buttonAddPressed();
    void buttonModifyPressed();
    void buttonDeletePressed();
  //..
    void dbMenuNew(void);
    void dbMenuOpen(void);
    void dbMenuRecent(QAction *);
    void dbMenuDelete(void);
    void dbMenuSave(void);
    void dbMenuSaveAs(void);
    void dbMenuQuit(void);
  //void editMenuCategories(void);
  //void editMenuCommands(void);
    void editMenuSettings(void);
  //void viewMenuNotes(void);
    void helpMenuIntroduction(void);
    void helpMenuAbout(void);
  //void exit(void);

  signals:
    void commandSelectedSignal(QString *);
    void terminalWindowInt_signal(TerminalWindow::BuiltInCmds,QStringList *);

  public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow(void);

    int getDBAccessResult(void) { return dbAccessResult; }
};

#endif // MAINWINDOW_H
