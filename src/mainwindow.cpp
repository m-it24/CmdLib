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

#include "mainwindow.h"

/**
 * @brief MainWindow::MainWindow
 *   Constructor of the main window. Calling the parent constructor with element initialisation
 *   (element initialisations are inserted between : and { ).
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), internVar(10)
{
  // get the operating system; not Windows => Linux
    if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows) {
        osType = WINDOWS;
    } else {
        osType = LINUX;
    }

  // setup the name of the configuration file and read the configuration
    cfgFile = "cmdlib.cfg";

    cfgAccess.setConfig(cfgFile);
    cfgAccess.readConfig();

    bool convOk;

  // get the position, width and height of the window from the configuration
    winPosX    = QString(cfgAccess.getValue("WINPOSX").c_str()).toInt(&convOk,10);
    winPosY    = QString(cfgAccess.getValue("WINPOSY").c_str()).toInt(&convOk,10);
    winWidth   = QString(cfgAccess.getValue("WINWIDTH").c_str()).toInt(&convOk,10);
    winHeight  = QString(cfgAccess.getValue("WINHEIGHT").c_str()).toInt(&convOk,10);
    notesWidth = QString(cfgAccess.getValue("NOTESWIDTH").c_str()).toInt(&convOk,10);

    // default size if there is no configuration
    if ((winWidth == 0) || (winHeight == 0)) {
        winWidth  = 800;
        winHeight = 600;
    }

    this->setGeometry(QRect(winPosX,winPosY,winWidth,winHeight));

  // setup the language
    language = cfgAccess.getValue("LANG");

    if (language.size() == 0) {
        language = "en";
    }

    //language = "de";

    if (language != "en") SettingsDialog::langInit(language);

    dbLast = cfgAccess.getValue("DBLAST");

  // open database for reading if it exists and contains the database identifier
    dbAccessResult = 0;

    // Is there a last used database in the configiration file?
    if (dbLast.size() > 0) {
      dbAccessResult = dbAccess.openRead(dbLast);
    }

    if (dbLast.size() == 0 || dbAccessResult != 0) {
        dbState = DB_NEW;
        dbName  = "New";
    } else {
        // database loaded
        dbState = DB_LOADED;

        getDBName();

        dbAccess.read();
        dbAccess.close();
    }

  // add the terminal and last command window to the central widget
    widgetCentral = new QWidget;

    gridCentralWidget = new QGridLayout;
    labelTerminal = createTerminalLabel();
    gridCentralWidget->addWidget(labelTerminal,0,0);
    textEditTerminal = createTerminalWindow();
    gridCentralWidget->addWidget(textEditTerminal,1,0);
    lineEditLastCommand = createLastCommandWindow();
    gridCentralWidget->addWidget(lineEditLastCommand,2,0);

    widgetCentral->setMinimumSize(320,200);
    widgetCentral->setLayout(gridCentralWidget);

    setCentralWidget(widgetCentral);

    // declaration of the signal commandEntered_signal in the TerminalWindow class;
    // the signal is emitted there in the method keyPressEvent
    connect(textEditTerminal,SIGNAL(commandEntered_signal(QString *)),this,SLOT(setCommandEntered(QString *)));

  // create the buttons for accessing the database
    docWidgetLeft = new QDockWidget(this);

    widgetDBAccessButtons = new QWidget;

    gridDBAccessButtons = new QGridLayout;

    buttonClr = new QPushButton(this);
    gridDBAccessButtons->addWidget(buttonClr,0,0);
    connect(buttonClr,SIGNAL(clicked()),this,SLOT(buttonClearPressed()));
    connect(this,SIGNAL(terminalWindowInt_signal(TerminalWindow::BuiltInCmds,QStringList *)),textEditTerminal,SLOT(commandInternal(TerminalWindow::BuiltInCmds,QStringList *)));

    buttonAdd = new QPushButton(this);
    gridDBAccessButtons->addWidget(buttonAdd,1,0);

    connect(buttonAdd,SIGNAL(clicked()),this,SLOT(buttonAddPressed()));

    buttonMod = new QPushButton(this);
    gridDBAccessButtons->addWidget(buttonMod,2,0);

    connect(buttonMod,SIGNAL(clicked()),this,SLOT(buttonModifyPressed()));

    buttonDel = new QPushButton(this);
    gridDBAccessButtons->addWidget(buttonDel,3,0);

    connect(buttonDel,SIGNAL(clicked()),this,SLOT(buttonDeletePressed()));

    widgetDBAccessButtons->setMinimumWidth(DBACCESSBUTTONSWIDTH);
    widgetDBAccessButtons->setMaximumWidth(DBACCESSBUTTONSWIDTH);
    widgetDBAccessButtons->setLayout(gridDBAccessButtons);

    docWidgetLeft->setWidget(widgetDBAccessButtons);
    docWidgetLeft->setAllowedAreas(Qt::LeftDockWidgetArea);

    addDockWidget(Qt::LeftDockWidgetArea,docWidgetLeft);

  // create the notes widget
    dockWidgetRight = new QDockWidget(this);

    dockWidgetRight->setAllowedAreas(Qt::RightDockWidgetArea);
    textEditCommandNotes = new CommandNotesWindow(notesWidth,0);  //createCommandNotesWindow();
    dockWidgetRight->setWidget(textEditCommandNotes);

    addDockWidget(Qt::RightDockWidgetArea,dockWidgetRight);

  // add a menu to the application
    menuBarMain = menuBar();

    menuDB = menuBarMain->addMenu("Menu 1");
    menuDBEntry1 = menuDB->addAction("Entry 1",this,SLOT(dbMenuNew()));
    menuDB->addSeparator();
    menuDBEntry2 = menuDB->addAction("Entry 2",this,SLOT(dbMenuOpen()));

    menuDBRecent = menuDB->addMenu("Submenu");

    // get the last used databases for the recent submenu
    for (int idx = 0; idx < 5; idx++) {
        string recentDB = cfgAccess.getValue(recentLabels[idx]);
        if (recentDB.size() > 0) {
            recentDBs.push_back(recentDB);
        } else {
            break;
        }
    }

    updateRecentDBs();

    connect(menuDBRecent,SIGNAL(triggered(QAction *)),this,SLOT(dbMenuRecent(QAction *)));

    menuDBEntry3 = menuDB->addAction("Entry 3",this,SLOT(dbMenuDelete()));
    menuDBEntry4 = menuDB->addAction("Entry 4",this,SLOT(dbMenuSave()));
    menuDBEntry5 = menuDB->addAction("Entry 5",this,SLOT(dbMenuSaveAs()));
    menuDB->addSeparator();
    menuDBEntry6 = menuDB->addAction("Entry 6",this,SLOT(dbMenuQuit()));

    menuEdit = menuBarMain->addMenu("Menu 2");
    menuEditEntry = menuEdit->addAction("Entry",this,SLOT(editMenuSettings()));
  /*menuEdit->addAction(tr("&Categories"),this,SLOT(editMenuCategories()));
    menuEdit->addAction(tr("C&ommands"),this,SLOT(editMenuCommands()));*/

  /*menuView = menuBarMain->addMenu(tr("&View"));
    menuView->addAction(tr("&Notes"),this,SLOT(viewMenuNotes()));*/

    menuHelp = menuBarMain->addMenu("Menu 3");
    menuHelpEntry1 = menuHelp->addAction("Entry 1",this,SLOT(helpMenuIntroduction()));
    menuHelp->addSeparator();
    menuHelpEntry2 = menuHelp->addAction("Entry 2",this,SLOT(helpMenuAbout()));

  // add a tool bar to the application
    toolBar = addToolBar("Toolbar");
    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    toolBar->setMovable(false);

    combCategories = new QComboBox;
    combCategories->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    toolBar->addWidget(combCategories);

    combCommands = new QComboBox;
    combCommands->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    toolBar->addWidget(combCommands);

    currCatNum = 0;
    setCategories();
    setCommands(currCatNum);

    connect(combCategories,SIGNAL(activated(int)),this,SLOT(setCommands(int)));
    connect(combCommands,SIGNAL(activated(int)),this,SLOT(setCommandSelected(int)));
    connect(this,SIGNAL(commandSelectedSignal(QString *)),textEditTerminal,SLOT(setCommandSelected(QString *)));

  // add a status line to the application
    statusBar = new QStatusBar;

    labelStatusBarLeft  = new QLabel;
    labelStatusBarRight = new QLabel;

    labelStatusBarLeft->setMinimumWidth(DBACCESSBUTTONSWIDTH+5);
    labelStatusBarLeft->setMaximumWidth(DBACCESSBUTTONSWIDTH+5);

    statusBar->addWidget(labelStatusBarLeft);
    statusBar->addWidget(labelStatusBarRight);

    setStatusBar(statusBar);

    translateMainWindow();  // translate the window texts to the configured language

    return;
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow(void)
{
  #ifdef DEBUG
    cout << "Destructor of main window called.\n" << flush;
  #endif

    // all widgets of the main window clear their child widgets, layouts and
    // all graphic elements the layouts contain
    if (widgetCentral!= nullptr) {
        delete widgetCentral;
        delete docWidgetLeft;
        delete dockWidgetRight;
        delete menuBarMain;
        delete toolBar;
        delete statusBar;
    }

    return;
}

/**
 * @brief MainWindow::sizeHint
 * @return
 */
//QSize MainWindow::sizeHint() const
//{
//    // is not called if the constructor calls the method setGeometry()
//    return QSize(winWidth/*800*/,winHeight/*600*/);
//}

/**
 * @brief translateMainWindow
 */
void MainWindow::translateMainWindow()
{
  // text for the terminal area
    setDBName(false);

  // texts for the command management
    docWidgetLeft->setWindowTitle(tr("Command"));
    buttonClr->setText(tr("&Clear"));
    buttonAdd->setText(tr("&Add"));
    buttonMod->setText(tr("&Modify"));
    buttonDel->setText(tr("De&lete"));

  // text for the note area
    dockWidgetRight->setWindowTitle(tr("Notes"));

  // texts for menu entries
    menuDB->setTitle(tr("&Database"));
    menuDBEntry1->setText(tr("&New"));
    menuDBEntry2->setText(tr("&Open"));
    menuDBEntry3->setText(tr("&Delete"));
    menuDBEntry4->setText(tr("&Save"));
    menuDBEntry5->setText(tr("Save &As"));
    menuDBEntry6->setText(tr("&Quit"));
    menuDBRecent->setTitle(tr("&Recent ..."));

    menuEdit->setTitle(tr("&Edit"));
    menuEditEntry->setText(tr("&Settings"));

    menuHelp->setTitle(tr("&Help"));
    menuHelpEntry1->setText(tr("&Introduction"));
    menuHelpEntry2->setText(tr("&About Command Library ..."));

    return;
}

/**
 * @brief getDBName
 */
void MainWindow::getDBName()
{
    int size = dbLast.size();
    int idx  = dbLast.find_last_of('/',size);

    if (idx < 0) {
      dbName.assign(dbLast,0,size);
      dbPath.clear();
    } else {
      dbName.assign(dbLast,idx+1,size-idx-1);
      dbPath.assign(dbLast,0,idx+1);
    }

    return;
}

/**
 * @brief MainWindow::setDBName
 * @param name
 * @param modified
 */
void MainWindow::setDBName(bool modified)
{
    QString name(tr("Database")+": ");

    name+= dbName.c_str();

    if (modified) {
        name+= " (*)";
    }

    labelStatusBarRight->setText(name);

    return;
}

/**
 * @brief MainWindow::resetDB
 */
void MainWindow::resetDB()
{
    dbAccess.clear();
    dbLast.clear();
    dbPath.clear();

    dbState = DB_NEW;
    dbName  = "New";

    currCatNum = 0;
    setCategories();
    setCommands(currCatNum);
    setDBName(false);

    QString lc;
    setCommandEntered(&lc);

    textEditCommandNotes->clear();

    return;
}

/**
 * @brief MainWindow::changeEvent
 * @param event
 */
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        //translateMainWindow();
    } else {
        this->QMainWindow::changeEvent(event);  // not handled events are passed to the parent class
    }

    return;
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event)/*exit()*/
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Command Library");
    msgBox.setIcon(QMessageBox::Information);

  // save the command library
    switch (dbState) {
        case DB_NEW_EDITED:
            msgBox.setText("Do you want to create a new database or\n"
                           "discard added commands?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            msgBox.setDefaultButton(QMessageBox::Ok);
            switch (msgBox.exec()) {
                case QMessageBox::Discard:
                    break;
                case QMessageBox::Ok:
                default:
                    displayHintSQLite();
                    dbLast = getDatabaseFile(QFileDialog::AcceptSave,QFileDialog::AnyFile).toStdString();
                    break;
            }
            break;
        case DB_MODIFIED:
            msgBox.setText("Save changes to database?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            msgBox.setDefaultButton(QMessageBox::Ok);
            switch (msgBox.exec()) {
                case QMessageBox::Discard:
                    dbLast.clear();
                    break;
                case QMessageBox::Ok:
                default:
                    break;
            }
            break;
        default:
            break;
    }

    // save the database and add the database to the configuration
    if (dbLast.size() > 0) {
        dbAccess.openWrite(dbLast);
        dbAccess.write();
        dbAccess.close();
        cfgAccess.setValue("DBLAST",dbLast);
    }

  // add the last used databases to the configuration
    iterStr iter = recentDBs.begin();

    for (int idx = 0; idx < 5; idx++) {
        if (iter != recentDBs.end()) {
            cfgAccess.setValue(recentLabels[idx],*iter);
            iter++;
        } else {
            cfgAccess.removeKey(recentLabels[idx]);
        }
    }

  // add the window position and size to the configuration and then save the configuration
  //QSize winSize = this->size();
    QRect winGeometry = this->geometry();
    cfgAccess.setValue("WINPOSX",QString::number(winGeometry.x(),10).toStdString());
    cfgAccess.setValue("WINPOSY",QString::number(winGeometry.y(),10).toStdString());
    cfgAccess.setValue("WINWIDTH",QString::number(winGeometry.width(),10).toStdString());
    cfgAccess.setValue("WINHEIGHT",QString::number(winGeometry.height(),10).toStdString());
    notesWidth = dockWidgetRight->width();
    cfgAccess.setValue("NOTESWIDTH",QString::number(notesWidth,10).toStdString());
    cfgAccess.setValue("LANG",language);
    cfgAccess.writeConfig();

    event->accept();  // close the application
  //event->ignore();  // the application is not closed

    return;
}

/****************************************************************************
 * Main window widgets
 ****************************************************************************/

/**
 * @brief MainWindow::createTerminalLabel
 *   Creates a label for setting the vertical position of the terminal window
 *   to start at the same position as the notes window.
 * @return terminal label
 */
QLabel *MainWindow::createTerminalLabel()
{
    QLabel *tl = new QLabel;

    tl->setMaximumHeight(6);
    return tl;
}

/**
 * @brief MainWindow::createTerminalWindow
 *   Creates the terminal window.
 * @return terminal window
 */
QTextEdit *MainWindow::createTerminalWindow()
{
    QTextEdit *tw = new TerminalWindow();
    return tw;
}

/**
 * @brief MainWindow::createLastCommandWindow
 *   Creates the last command line.
 * @return last command line
 */
QLineEdit *MainWindow::createLastCommandWindow()
{
    QLineEdit *lcw = new QLineEdit;
    return lcw;
}

/**
 * @brief MainWindow::createCommandNotesWindow
 *   Creates the notes window.
 * @return notes window
 */
//QTextEdit *MainWindow::createCommandNotesWindow()
//{
//    QTextEdit *cnw = new QTextEdit;
//    return cnw;
//}

/****************************************************************************
 * Slot methods for the command management
 ****************************************************************************/

/**
 * @brief MainWindow::buttonClearPressed
 *   Clears the terminal window, the last command line and the notes window.
 */
void MainWindow::buttonClearPressed()
{
    // clear the terminal window
    emit terminalWindowInt_signal(TerminalWindow::BuiltInCmds::CLEAR,new QStringList(QStringList() << "clear"));

    textEditTerminal->setFocus(Qt::OtherFocusReason);  // set the focus to the terminal window

    // clear the last command line
    lineEditLastCommand->clear();

    // clear the notes window
    textEditCommandNotes->clear();
    return;
}

/**
 * @brief MainWindow::buttonAddPressed
 */
void MainWindow::buttonAddPressed()
{
    QMessageBox *msgBox = new QMessageBox(this);

    msgBox->setWindowTitle(tr("Add Command"));

    if (lineEditLastCommand->text().size() == 0/*lastCmd.empty()*/) {
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(tr("No command entered for adding!"));
        msgBox->exec();
        delete msgBox;
        return;
    }

    AddDialog *addDialog = new AddDialog(new QString(lineEditLastCommand->text()/*lastCmd.c_str()*/),catList,this);

    if (addDialog->exec()) {

        QString command  = addDialog->getCommand();
        QString category = addDialog->getCategory();

        int cmdExistsCntr = 0;

        iterStr iter = catList.begin();

        // find the choosen category in the category list
        for (currCatNum = 0; currCatNum < (int)catList.size(); currCatNum++) {
            if (iter->c_str() == category.toStdString()) break;
            iter++;
        }

        // New category entered?
        if (iter == catList.end()) {
            catList.push_back(category.toStdString());
            combCategories->addItem(catList.back().c_str());  // catList.back() returns a reference on the string
        } else {
            cmdExistsCntr++;

            list<string> cmdListCat = dbAccess.cmdRead(category.toStdString());

            // find the added command in the command list;
            // std:: due to qwidget contains a static find method
            iter = std::find(cmdListCat.begin(),cmdListCat.end(),command.toStdString());

            // New command entered?
            if (iter != cmdListCat.end()) {
                cmdExistsCntr++;
            }
        }

        // Category or command doesn't exist?
        if (cmdExistsCntr < 2) {

            list<string> cmd;

            cmd.push_back(category.toStdString());
            cmd.push_back(command.toStdString());

            QStringList notes = textEditCommandNotes->toPlainText().split('\n',Qt::KeepEmptyParts,Qt::CaseInsensitive);

            int size = notes.size();  // Calling notes.size() in the loop is not possible due to notes.size() is counted down after calling notes.takeAt().

            for (int pos = 0; pos < size; pos++) {
                cmd.push_back(notes.takeAt(0).toStdString()/*+'\n'*/);
            }

            dbAccess.cmdAdd(cmd);
            setCommands(currCatNum);  // Show command in the command list.
            combCommands->setCurrentIndex(cmdList.size()-1);
            setDBName(true);

            switch (dbState) {
                case DB_LOADED: dbState = DB_MODIFIED;   break;
                case DB_NEW:    dbState = DB_NEW_EDITED; break;
                default:
                    break;
            }
        } else {
            msgBox->setIcon(QMessageBox::Information);
            msgBox->setText(tr("Command not added!")+"\n\n"
                           +tr("Category")+" '"+category+"' "+tr("already contains the command.")+"\n");
            msgBox->exec();
        }
    }

    delete addDialog;
    delete msgBox;
    return;
}

/**
 * @brief MainWindow::buttonModifyPressed
 */
void MainWindow::buttonModifyPressed()
{
    int size;
    list<string> cmdLst;
    QStringList  notes;

    QMessageBox *msgBox = new QMessageBox(this);

    msgBox->setWindowTitle(tr("Modify Command"));

    if (currCmd.empty() || lineEditLastCommand->text().isEmpty()/*lastCmd.empty()*/) {
        msgBox->setIcon(QMessageBox::Information);
        if (currCmd.empty()) {
            msgBox->setText(tr("No command selected for modifying!"));
        } else {
            msgBox->setText(tr("There is no modification of the command!"));
        }
        msgBox->exec();
        delete msgBox;
        return;
    }

    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setText(tr("Are you sure to modify")+" ...");
    msgBox->setInformativeText("> "+QString(currCmd.c_str())+" "+tr("to")+" ...\n"
                              +"> "+lineEditLastCommand->text()/*QString(lastCmd.c_str())*/+" ?");
    msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    int retVal = msgBox->exec();

    switch (retVal) {
        case QMessageBox::Ok:
            cmdLst.push_back(currCat);
            cmdLst.push_back(currCmd);
            cmdLst.push_back(lineEditLastCommand->text().toStdString()/*lastCmd*/);

            notes = textEditCommandNotes->toPlainText().split('\n',Qt::KeepEmptyParts,Qt::CaseInsensitive);

            size = notes.size();  // Aufruf in der Schleife nicht möglich, da notes.size() nach notes.takeAt() heruntergezählt wird.

            for (int pos = 0; pos < size; pos++) {
                cmdLst.push_back(notes.takeAt(0).toStdString()/*+'\n'*/);
            }

            retVal = dbAccess.cmdModify(cmdLst);

            if (retVal > 0) {
                msgBox->setIcon(QMessageBox::Warning);
                msgBox->setText(tr("Error in modyfing command!"));
            } else {
                msgBox->setIcon(QMessageBox::Information);
                msgBox->setText(tr("Command successful modified."));

                setCommands(currCatNum);
                combCommands->setCurrentIndex(currCmdNum);
                setDBName(true);

                switch (dbState) {
                    case DB_LOADED: dbState = DB_MODIFIED; break;
                    default:
                        break;
                }
            }

            msgBox->setInformativeText("");
            msgBox->setStandardButtons(QMessageBox::Ok);
            msgBox->exec();
            break;
        case QMessageBox::Cancel:
        default:
            break;
    }

    delete msgBox;
    return;
}

/**
 * @brief MainWindow::buttonDeletePressed
 */
void MainWindow::buttonDeletePressed()
{
    list<string> cmdLst;

    QMessageBox *msgBox = new QMessageBox(this);

    msgBox->setWindowTitle(tr("Delete Command"));

    if (currCmd.empty()) {
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(tr("No command selected for deleting!"));
        msgBox->exec();
        delete msgBox;
        return;
    }

    QString info(currCmd.c_str());
    QString spaces("");

    spaces.fill(' ',info.size()+4);

    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setText(tr("Are you sure to delete")+" ..."+spaces);
    msgBox->setInformativeText("> "+info+" ?");
    msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    int retVal = msgBox->exec();

    switch (retVal) {
        case QMessageBox::Ok:
            cmdLst.push_back(currCat);
            cmdLst.push_back(currCmd);

            retVal = dbAccess.cmdDelete(cmdLst);

            if (retVal > 0) {
                msgBox->setIcon(QMessageBox::Warning);
                msgBox->setText(tr("Error in deleting command!"));
            } else {
                msgBox->setIcon(QMessageBox::Information);
                msgBox->setText(tr("Command successful deleted."));

                // Last command from the category deleted?
                if (cmdList.size()-1 == 0) {
                    if (currCatNum > 0) currCatNum--;
                    setCategories();
                }

                currCmd.clear();
                setCommands(currCatNum);
                setDBName(true);

                switch (dbState) {
                    case DB_LOADED: dbState = DB_MODIFIED; break;
                    default:
                        break;
                }
            }

            msgBox->setInformativeText("");
            msgBox->setStandardButtons(QMessageBox::Ok);
            msgBox->exec();

            break;
        case QMessageBox::Cancel:
        default:
            break;
    }

    delete msgBox;
    return;
}

/****************************************************************************
 * Processing categories and commands
 ****************************************************************************/

/**
 * @brief MainWindow::setCategories
 *   Creates a list with the available categories and adds the categories
 *   to the categories combo box.
 */
void MainWindow::setCategories(void)
{
    catList = dbAccess.catRead();

    combCategories->clear();

    for (iterStr iter = catList.begin(); iter!= catList.end(); iter++) {
        combCategories->addItem(iter->c_str());
    }

    return;
}

/**
 * @brief MainWindow::setCommands
 *   Creates a list with the available commands of a choosen category and
 *   adds the commands to the commands combo box.
 * @param cat
 */
void MainWindow::setCommands(int cat)
{
    // Are there any categories?
    if (!catList.empty()) {

        iterStr iter = catList.begin();

        for (int cntr = 0; cntr < cat; cntr++) {
            iter++;
        }

        currCat = iter->c_str();
        cmdList = dbAccess.cmdRead(currCat);  // read the commands of the category

        combCategories->setCurrentIndex(cat);
    } else {
        currCat.clear();
        currCmd.clear();
        cmdList.clear();
    }

    combCommands->clear();

    for (iterStr iter = cmdList.begin(); iter!= cmdList.end(); iter++) {
        combCommands->addItem(iter->c_str());
    }

    currCatNum = cat;  // is set too here, because the method is a slot of the category choice

    return;
}

/**
 * @brief MainWindow::setCommandSelected
 * @param cmd
 */
void MainWindow::setCommandSelected(int cmd)
{
    iterStr iter = cmdList.begin();

    for (int cntr = 0; cntr < cmd; cntr++) {
        iter++;
    }

    currCmd = iter->c_str();
    QString str(currCmd.c_str());

    emit commandSelectedSignal(&str/*new QString(currCmd.c_str())*/);

    list<string> notes = dbAccess.ntsRead(currCat,currCmd);

    textEditCommandNotes->clear();

    while (!notes.empty()) {
        str = notes.front().c_str();
        notes.pop_front();
        textEditCommandNotes->append(str);
    }

    textEditTerminal->setFocus();
    currCmdNum = cmd;

    return;
}

/**
 * @brief MainWindow::setCommandEntered
 * @param cmd
 */
void MainWindow::setCommandEntered(QString *cmd)
{
    // Close the program with the exit command?
    if (*cmd != "exit") {
      //lastCmd = cmd->toStdString();
        lineEditLastCommand->setText(*cmd);
    } else {
        close();
    }

    return;
}

/****************************************************************************
 * Processing the database menu
 ****************************************************************************/

/**
 * @brief MainWindow::displayHintSQLite
 */
void MainWindow::displayHintSQLite()
{
    string dontShow;
    QMessageBox msgBox;

    if (cfgAccess.getValue("HINTSQLITE") != "false") {

        msgBox.setWindowTitle("Command Library");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("To create a SQLite Database use .sqlite as file extension.")+"\n"
                      +tr("Using other extensions creates a text database."));

        QPushButton *btnOk = msgBox.addButton("Ok",QMessageBox::YesRole);
        QPushButton *btnDontShow = msgBox.addButton(tr("Don't show again"),QMessageBox::NoRole);

        msgBox.setDefaultButton(btnOk);
        msgBox.exec();

        if (msgBox.clickedButton() == btnDontShow) {
            dontShow = "false";
        } else {
            dontShow = "true";
        }

        cfgAccess.setValue("HINTSQLITE",dontShow);
    }

    return;
}

/**
 * @brief getDatabaseFile
 * @param fileMode
 * @return
 */
QString MainWindow::getDatabaseFile(QFileDialog::AcceptMode acceptMode, QFileDialog::FileMode fileMode)
{
    QString filename;
    QFileDialog fileDialog(this);

    fileDialog.setAcceptMode(acceptMode);
    fileDialog.setFileMode(fileMode);
    fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
    fileDialog.setNameFilter("Database (*.cly *.sqlite)");

    if (fileDialog.exec()) {
        QStringList filenames = fileDialog.selectedFiles();
        filename = filenames[0];
    }

    return filename;
}

/**
 * @brief MainWindow::dbMenuNew
 *   The menu entry 'New' resets the command list to create
 *   a new database.
 */
void MainWindow::dbMenuNew()
{
    string dbFile;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Command Library");
    msgBox.setIcon(QMessageBox::Information);

  #ifdef DEBUG
    cout << "Menu entry 'Database/New' called.\n" << flush;
  #endif

    switch (dbState) {
        case DB_NEW_EDITED:
            msgBox.setText("Commands added not saved to a database!\n"
                           "Do you want to save these commands before\n"
                           "creating a new database?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            msgBox.setDefaultButton(QMessageBox::Ok);
            switch (msgBox.exec()) {
                case QMessageBox::Discard:
                    break;
                case QMessageBox::Ok:
                default:
                    displayHintSQLite();
                    dbFile = getDatabaseFile(QFileDialog::AcceptSave,QFileDialog::AnyFile).toStdString();
                    break;
            }
            break;
        case DB_MODIFIED:
            msgBox.setText("Save changes to the database?\n");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            msgBox.setDefaultButton(QMessageBox::Ok);
            switch (msgBox.exec()) {
                case QMessageBox::Discard:
                    break;
                case QMessageBox::Ok:
                default:
                    dbFile = dbLast;
                    break;
            }
            break;
        default:
            break;
    }

  // save the current used database
    if (dbFile.size() > 0) {
        dbAccess.openWrite(dbFile);
        dbAccess.write();
        dbAccess.close();
        cfgAccess.setValue("DBLAST",dbFile);

        addRecentDB(dbFile);
        updateRecentDBs();
    }

  // reset the command list and clear the graphic elements
    resetDB();

    return;
}

/**
 * @brief MainWindow::dbMenuOpen
 *   The menu entry 'Open' opens an existing database.
 */
void MainWindow::dbMenuOpen()
{
    QString dbFile;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Command Library");
    msgBox.setIcon(QMessageBox::Information);

  #ifdef DEBUG
    cout << "Menu entry 'Database/Open' called.\n" << flush;
  #endif

    switch (dbState) {
        case DB_NEW_EDITED:
            msgBox.setText("Commands added not saved to a database!\n"
                           "Do you want to save these commands before\n"
                           "opening a database?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            msgBox.setDefaultButton(QMessageBox::Ok);
            switch (msgBox.exec()) {
                case QMessageBox::Discard:
                    break;
                case QMessageBox::Ok:
                default:
                    displayHintSQLite();
                    dbFile = getDatabaseFile(QFileDialog::AcceptSave,QFileDialog::AnyFile);
                    if (dbFile.size() > 0) {
                        dbLast  = dbFile.toStdString();
                        dbState = DB_LOADED;  // if 'Cancel' is choosen in the open dialog
                    }
                    break;
            }
            break;
        case DB_MODIFIED:
            msgBox.setText("Save changes to the database?\n");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            msgBox.setDefaultButton(QMessageBox::Ok);
            switch (msgBox.exec()) {
                case QMessageBox::Discard:
                    break;
                case QMessageBox::Ok:
                default:
                    dbFile  = dbLast.c_str();
                    dbState = DB_LOADED;
                    break;
            }
            break;
        default:
            break;
    }

  // save the current used database
    if (dbFile.size() > 0) {
        dbAccess.openWrite(dbLast);
        dbAccess.write();
        dbAccess.close();
        getDBName();
        setDBName(false);
    }

  // choose a database to open it
    if (recentDB.size() > 0) {
        dbFile   = recentDB;
        recentDB = "";
    } else {
        dbFile = getDatabaseFile(QFileDialog::AcceptOpen,QFileDialog::ExistingFile);
    }

    if (dbFile.size() > 0) {

      // add the last used database to the recent list
        if ((dbState != DB_NEW) && (dbState != DB_NEW_EDITED)) {
            addRecentDB(dbLast);
            updateRecentDBs();
        }

      // open the choosen database
        dbLast = dbFile.toStdString();

        dbAccess.openRead(dbLast);
        dbAccess.read();
        dbAccess.close();

        dbState = DB_LOADED;

        currCatNum = 0;
        setCategories();
        setCommands(currCatNum);
        getDBName();
        setDBName(false);

        QString lc;
        setCommandEntered(&lc);

        textEditCommandNotes->clear();
    }

    return;
}

/**
 * @brief MainWindow::dbMenuRecent
 */
void MainWindow::dbMenuRecent(QAction *action)
{
  #ifdef DEBUG
    cout << "Menu entry 'Database/Recent' called.\n" << flush;
  #endif

    recentDB = action->text();  // get the filename of the database to open
    recentDB.remove(0,3);       // remove the numeration at the begining of the filename

    if (!QFile::exists(recentDB)) {
      // display an error message
        QMessageBox *msgBox = new QMessageBox(this);

        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setText("Error! File not found ...");
        msgBox->setInformativeText(recentDB+" !");
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
        delete msgBox;

      // remove the entry from the recent list
        iterStr iter = recentDBs.begin();

        while (iter != recentDBs.end()) {
            if (*iter == recentDB.toStdString()) {
                recentDBs.erase(iter);
                break;
            }
            iter++;
        }

        updateRecentDBs();
    } else {
        dbMenuOpen();
    }

    return;
}

/**
 * @brief MainWindow::addRecentDB
 * @param dbFile
 */
void MainWindow::addRecentDB(string dbFile)
{
    iterStr iter = recentDBs.begin();

  // remove the database from the recent list if it is already there
  // to place the database at the begining of the recent list
    while (iter != recentDBs.end()) {
        if (*iter == dbFile) {
            recentDBs.erase(iter);
            break;
        }
        iter++;
    }

  // at most five entries in the recent list
    if (recentDBs.size() >= 5) {
        recentDBs.pop_back();
    }

  // place the database at the begining of the recent list
    recentDBs.push_front(dbFile);

    return;
}

/**
 * @brief MainWindow::updateRecentDBs
 */
void MainWindow::updateRecentDBs() {

    menuDBRecent->clear();  // clear the recent list

    iterStr iter = recentDBs.begin();

  // add the recently used databases to the recent sub menu
    int idx = 1;

    while (iter != recentDBs.end()) {
        string entry = "&";
        entry.append(1,'0'+idx);
        entry+= " "+*iter;

        QAction *action = new QAction(entry.c_str(),this);
        menuDBRecent->addAction(action);
      //menuDBRecent->addAction(entry.c_str(),this,SLOT(dbMenuRecent(QAction *)));  // doesn't work, no call of dbMenuRecent()

        // add the next recently used database
        idx++;
        iter++;
    }

    return;
}

/**
 * @brief MainWindow::dbMenuDelete
 */
void MainWindow::dbMenuDelete()
{
  #ifdef DEBUG
    cout << "Menu entry 'Database/Delete' called.\n" << flush;
  #endif

    QDir path;
    QMessageBox *msgBox = new QMessageBox(this);

    msgBox->setWindowTitle(tr("Delete database"));
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setText(tr("Are you sure to delete")+" "
                   +QString(dbName.c_str())
                   +" ?");
    msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    int retVal = msgBox->exec();

    switch (retVal) {
        case QMessageBox::Ok:
            path.setPath(dbPath.c_str());
            path.remove(dbName.c_str());
            resetDB();  // reset the command list and clear the graphic elements
            break;
        case QMessageBox::Cancel:
        default:
            break;
    }

    delete msgBox;
    return;
}

/**
 * @brief MainWindow::dbMenuSave
 */
void MainWindow::dbMenuSave()
{
  #ifdef DEBUG
    cout << "Menu entry 'Database/Save' called.\n" << flush;
  #endif

    switch (dbState) {
        case DB_NEW:
        case DB_NEW_EDITED:
            dbMenuSaveAs();
            break;
        case DB_LOADED:
        case DB_MODIFIED:
        default:
            dbState = DB_LOADED;

            dbAccess.openWrite(dbLast);
            dbAccess.write();
            dbAccess.close();
            setDBName(false);
            break;
    }

    return;
}

/**
 * @brief MainWindow::dbMenuSave
 */
void MainWindow::dbMenuSaveAs()
{
  #ifdef DEBUG
    cout << "Menu entry 'Database/Save As' called.\n" << flush;
  #endif

    displayHintSQLite();
    QString dbFile = getDatabaseFile(QFileDialog::AcceptSave,QFileDialog::AnyFile);

    if (dbFile.size() > 0) {
        dbState = DB_LOADED;
        dbLast  = dbFile.toStdString();

        dbAccess.openWrite(dbLast);
        dbAccess.write();
        dbAccess.close();
        getDBName();
        setDBName(false);
    }

    return;
}

/**
 * @brief MainWindow::dbMenuQuit
 */
void MainWindow::dbMenuQuit()
{
  #ifdef DEBUG
    cout << "Menu entry 'Database/Quit' called.\n" << flush;
  #endif

    close();  // close the main window
    return;
}

/****************************************************************************
 * Processing the edit menu
 ****************************************************************************/

/**
 * @brief MainWindow::editMenuCategories
 */
//void MainWindow::editMenuCategories()
//{
//  #ifdef DEBUG
//    cout << "Menu entry 'Edit/Categories' called.\n" << flush;
//  #endif
//    return;
//}

/**
 * @brief MainWindow::editMenuCommands
 */
//void MainWindow::editMenuCommands()
//{
//  #ifdef DEBUG
//    cout << "Menu entry 'Edit/Commands' called.\n" << flush;
//  #endif
//    return;
//}

void MainWindow::editMenuSettings()
{
    SettingsDialog::Settings *settings = new SettingsDialog::Settings;

  #ifdef DEBUG
    cout << "Menu entry 'Edit/Settings' called.\n" << flush;
  #endif

    settings->language = language;

    SettingsDialog *settingsDialog = new SettingsDialog(settings,this);

    if (settingsDialog->exec()) {
        language = settings->language;
        translateMainWindow();
    }

    delete settings;
    delete settingsDialog;
    return;
}

/****************************************************************************
 * Processing the view menu
 ****************************************************************************/

/**
 * @brief MainWindow::viewMenuNotes
 */
//void MainWindow::viewMenuNotes()
//{
//  #ifdef DEBUG
//    cout << "Menu entry 'View/Notes' called.\n" << flush;
//  #endif
//    return;
//}

/****************************************************************************
 * Processing the help menu
 ****************************************************************************/

/**
 * @brief MainWindow::helpMenuIntroduction
 */
void MainWindow::helpMenuIntroduction(void)
{
  #ifdef DEBUG
    cout << "Menu entry 'Help/Introduction' called.\n" << flush;
  #endif

    introductionWindow = new IntroWindow(language,this/*nullptr*/,Qt::Window);
    return;
}

/**
 * @brief MainWindow::helpMenuAbout
 *   Opens a window with program information.
 */
void MainWindow::helpMenuAbout()
{
  #ifdef DEBUG
    cout << "Menu entry 'Help/About Command Library ...' called.\n" << flush;
  #endif

    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setModal(true);
    aboutDialog->setWindowTitle(tr("About"));
    aboutDialog->setFixedSize(QSize(280,150));

    QBoxLayout *aboutLayout = new QBoxLayout(QBoxLayout::TopToBottom,aboutDialog);
    aboutLayout->addWidget(new QLabel("Command Library"),0,Qt::AlignHCenter);
    aboutLayout->addWidget(new QLabel("Version 0.03"),0,Qt::AlignHCenter);
    aboutLayout->addWidget(new QLabel(tr("Written by")+" Rainer Otto (2024)"),0,Qt::AlignTop|Qt::AlignHCenter);
    QPushButton *okButton = new QPushButton("Ok");
    aboutLayout->addWidget(okButton,0,Qt::AlignHCenter);

    connect(okButton,SIGNAL(clicked()),aboutDialog,SLOT(accept()));

    aboutDialog->setLayout(aboutLayout);
    aboutDialog->exec();

    delete aboutDialog;
    return;
}

/****************************************************************************
 * Implementation of the command notes window
 ****************************************************************************/

/**
 * @brief MainWindow::CommandNotesWindow::CommandNotesWindow
 * @param w
 * @param h
 */
MainWindow::CommandNotesWindow::CommandNotesWindow(int w, int h) {
    width  = w;
    height = h;
    return;
}

/**
 * @brief MainWindow::CommandNotesWindow::sizeHint
 * @return
 */
QSize MainWindow::CommandNotesWindow::sizeHint() const
{
    return QSize(width,height);
}
