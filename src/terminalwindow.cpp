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

#include "terminalwindow.h"

using namespace std;

namespace lv {  // namespace for loop variables
    int pos;
}

/**
 * @brief TerminalWindow::TerminalWindow
 *   Constructor of the class TerminalWindow.
 * @param parent
 */
TerminalWindow::TerminalWindow(QWidget *parent) : QTextEdit(parent)
{
    QTextCharFormat *charFormat = new QTextCharFormat();

    QFont *termFont = new QFont(/*"Mono""Monospace""Bitstream Vera Sans Mono""DejaVu Sans Mono"*/"Nimbus Mono PS",  // font family
                                10,      // point size
                                -1,      // weight
                                false);  // italic

    charFormat->setFont(*termFont);

    currCursor = textCursor();  // textCursor() only returns a copy of QTextEdit's text cursor

    currCursor.setCharFormat(*charFormat);
    currCursor.insertText("cmd$ ");

    // The following setTextCursor() call is needed only, if there are changes to the text cursor attributes.
    // The copy of QTextEdit's text cursor works on the same document as the QTextEdit's text cursor.
    setTextCursor(currCursor);

    cmdLineStart = currCursor.position();
    cmdLineEnd   = cmdLineStart;
    cmdEntered   = new QString;
    process      = new QProcess(this);
    homeDir      = QDir::homePath()/*"c:/Users/R.Otto"*/;

    homeDir.replace(0,1,homeDir.at(0).toUpper());

    workDir = new QDir(homeDir);

    process->setWorkingDirectory(homeDir);  // set the working directory

    // Get the drive of the home directory under Windows;
    // under Linux the drive is the root directory.
    wrkDrive = homeDir.at(0);

    connect(this,SIGNAL(commandInt_signal(TerminalWindow::BuiltInCmds,QStringList*)),this,SLOT(commandInternal(TerminalWindow::BuiltInCmds,QStringList*)));
    connect(this,SIGNAL(commandExt_signal(QStringList*)),this,SLOT(commandExternal(QStringList*)));

    connect(process,SIGNAL(started()),this,SLOT(commandStarted()));
    connect(process,SIGNAL(errorOccurred(QProcess::ProcessError)),this,SLOT(commandError(QProcess::ProcessError)));
    connect(process,SIGNAL(channelReadyRead(int)),this,SLOT(commandReadyRead(int)));
    connect(process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(commandFinished(int,QProcess::ExitStatus)));

    return;
}

/**
 * @brief TerminalWindow::keyPressEvent
 *   The method is called if a key is pressed in the terminal window.
 * @param event
 */
void TerminalWindow::keyPressEvent(QKeyEvent *event)
{
    int         cmdLineCurr;
    BuiltInCmds cmdBuiltIn;  // in C++ the keyword enum is not needed in front of the enumeration name
    QChar       cmdChar;
    QStringList *cmdParts;

    //cout << posCmdLine << "  ";
    //cout.flush();

    currCursor  = textCursor();
    cmdLineCurr = currCursor.position();
    cmdBuiltIn  = NONE;

    switch (event->key()) {
        case Qt::Key_Backspace:
        case Qt::Key_Left:
            if (cmdLineCurr > cmdLineStart) QTextEdit::keyPressEvent(event);
            break;
        case Qt::Key_Up:
            break;
        case Qt::Key_Home:
            currCursor.setPosition(cmdLineStart);
            setTextCursor(currCursor);
            break;
        case Qt::Key_Return:
            // call of the basis class implementation due to finish the line with the entered return
            QTextEdit::keyPressEvent(event);

            cmdLineEnd = document()->characterCount() - 1;  // position of the last character = size of the document - 1

            if (cmdLineStart < (cmdLineEnd-1)) {
                cmdEntered->clear();
                for (int pos = cmdLineStart; pos < cmdLineEnd; pos++) {
                    cmdChar = document()->characterAt(pos);
                    if (cmdChar.unicode() != 0x2029) {  // 0x2029 = paragraph separator to the next line
                        cmdEntered->append(cmdChar);
                    }
                }
                // send main window the entered command
                emit commandEntered_signal(cmdEntered);

                // The tilde (~) for the home directory used in the pathname?
                if (cmdEntered->contains('~')) {
                    int pos = cmdEntered->indexOf('~');
                    cmdEntered->remove(pos,1);
                    cmdEntered->insert(pos,homeDir);
                }

                cmdParts = getCommandParts(cmdEntered);

                // Built in command entered?
                if (cmdParts->at(0) == "cd")    { cmdBuiltIn = CD; }
                if (cmdParts->at(0) == "clear") { cmdBuiltIn = CLEAR; }
                if (cmdParts->at(0) == "exit")  { cmdBuiltIn = EXIT; }

                if (cmdBuiltIn != NONE) {
                    emit commandInt_signal(cmdBuiltIn,cmdParts);
                } else {
                    emit commandExt_signal(cmdParts);
                }
            } else {
                // No command entered and return is pressed, then
                // a new prompt is displayed on the next line.
                currCursor.insertText("cmd$ ");

                setTextCursor(currCursor);

                cmdLineStart = currCursor.position();
                cmdLineEnd   = cmdLineStart;
            }
            break;
        default:
            QTextEdit::keyPressEvent(event);
            break;
    }

    return;
}

/**
 * @brief TerminalWindow::setCommandSelected
 *   The method is called if a command is choosen from the command list.
 * @param cmd
 */
void TerminalWindow::setCommandSelected(QString *cmd)
{
    currCursor = textCursor();
    cmdLineEnd = document()->characterCount() - 1;  // position of the last character = size of the document - 1
    currCursor.setPosition(cmdLineEnd);

    // Is there a command still next to the prompt?
    while (cmdLineEnd > cmdLineStart) {
        currCursor.deletePreviousChar();
        cmdLineEnd--;
    }

    currCursor.insertText(*cmd);
    setTextCursor(currCursor);
    return;
}

/**
 * @brief TerminalWindow::getCommandParts
 *   Identifies command and arguments from the entered command line in the terminal window.
 * @param cmd  entered command line
 * @return list containing the command and arguments
 */
QStringList *TerminalWindow::getCommandParts(QString *cmd)
{
    QChar       ch;
    QString     part;  // defines a null string
    QStringList *parts = new QStringList;  // list for the command line parts

  #ifdef DEBUG
    cout << "Execute command.\n";
    cout.flush();
  #endif

    for (lv::pos = 0; lv::pos < cmd->size(); lv::pos++) {
        ch = cmd->at(lv::pos);
        if (ch != ' ') part.append(ch);
        if ((ch == ' ') || (lv::pos >= (cmd->size()-1))) {
            *parts << part;
            part.clear();
        }
    }

    return parts;
}

/**
 * @brief TerminalWindow::commandInternal
 *   Executes commands build in Qt or the application.
 * @param cmd
 * @param cmdParts
 */
void TerminalWindow::commandInternal(BuiltInCmds cmd, QStringList *cmdParts)
{
    bool result;
    bool dspDir = false;
    QString pathName;

    currCursor = textCursor();

    switch (cmd) {
        case CD:
            if (cmdParts->size() > 1) {
                cmdParts->pop_front();
                pathName = cmdParts->join(' ');
                pathName = QDir::cleanPath(pathName);  // contains pathName.replace("\\","/") and other cleanings
                if (pathName.at(0) == '"') pathName.remove(0,1);
                if (pathName.at(pathName.size()-1) == '"') pathName.remove(pathName.size()-1,1);
                if (pathName.startsWith("..")) dspDir = true;
                // under Windows add the drive letter in front of the absolute pathname
                if (wrkDrive != '/' && pathName.startsWith("/")) {
                    pathName = QString(wrkDrive) + ":" + pathName;
                }
                // use uppercase drive letters for workDir
                if (wrkDrive != '/' && pathName.at(1) == ':') {
                    pathName.replace(0,1,pathName.at(0).toUpper());
                }
            } else {
                pathName = homeDir;
                dspDir   = true;
            }

            result = workDir->cd(pathName);

            if (result) {
                pathName = workDir->absolutePath();
                wrkDrive = pathName.at(0);
                process->setWorkingDirectory(pathName);
                if (dspDir) {
                    currCursor.insertText(pathName+"\n");
                }
            } else {
                currCursor.insertText(tr("Directory not found!")+"\n");
            }
            break;
        case CLEAR: {
            currCursor = textCursor();
            currCursor.setPosition(0);
            QTextCharFormat cf = currCursor.charFormat();
            this->clear();
            currCursor.setCharFormat(cf);
            setTextCursor(currCursor); }
            break;
        case EXIT:
            break;
        case NONE:
        default:
            break;
    }

    currCursor.insertText("cmd$ ");
    cmdLineStart = currCursor.position();
    cmdLineEnd   = cmdLineStart;

    setTextCursor(currCursor);

    return;
}

/**
 * @brief TerminalWindow::commandExternal
 *   Executes commands as processes.
 * @param cmdParts
 */
void TerminalWindow::commandExternal(QStringList *cmdParts)
{
    bool result    = true;
    bool dspPrompt = false;
    QString command;
    QString argCmd;
    QString argLine;
    QStringList arguments;

    currCursor = textCursor();

    if (wrkDrive == '/') {
        // execute a command under Linux
        command = cmdParts->at(0);
        for (lv::pos = 1; lv::pos < cmdParts->size(); lv::pos++) {
            arguments << cmdParts->at(lv::pos);
        }
        process->start(command,arguments);
    } else {
        // execute a command under Windows
        argCmd = cmdParts->at(0);
        // Switch the drive?
        if (argCmd.at(1) == ':' && argCmd.size() == 2) {
            argCmd = QString(argCmd.at(0).toUpper()) + ":/";
            result = workDir->cd(argCmd);
            if (!result) {
                currCursor.insertText(tr("Drive not available!")+"\n");
            } else {
                wrkDrive = argCmd.at(0);
                process->setWorkingDirectory(argCmd);
                currCursor.insertText(workDir->absolutePath()+"\n");
            }
            dspPrompt = true;
        } else {
            command = "cmd";
            arguments << "/C";
            arguments << argCmd;
            cmdParts->pop_front();
            argLine = cmdParts->join(' ');
            if (argLine.size() > 0) {
                if (argLine.at(0) == '"') argLine.remove(0,1);
                if (argLine.at(argLine.size()-1) == '"') argLine.remove(argLine.size()-1,1);
                arguments << argLine;
            }
            process->start(command,arguments);
        }
    }

    // Display the prompt again?
    if (dspPrompt) {
        currCursor.insertText("cmd$ ");
        cmdLineStart = currCursor.position();
        cmdLineEnd   = cmdLineStart;
        setTextCursor(currCursor);
    }

    return;
}

/**
 * @brief TerminalWindow::commandStarted
 *   Is called when the process is started. Then the process is in the running state.
 */
void TerminalWindow::commandStarted()
{
  #ifdef DEBUG
    cout << "Command started.\n";
    cout.flush();
  #endif
    return;
}

/**
 * @brief TerminalWindow::commandError
 *   Is called if there is an error during the execution.
 * @param error
 */
void TerminalWindow::commandError(QProcess::ProcessError error)
{
    string errorMsg;

    errorMsg = "Error in executing command (";

    if (error == 0) errorMsg+= "0=failed to start";
    else if (error == 1) errorMsg+= "1=crashed";
    else if (error == 2) errorMsg+= "2=timed out";
    else if (error == 3) errorMsg+= "3=read error";
    else if (error == 4) errorMsg+= "4=write error";
    else if (error == 5) errorMsg+= "5=unknown error";

    errorMsg+= ")!\n";

    currCursor = textCursor();

    currCursor.insertText(QString(errorMsg.c_str()));
    currCursor.insertText("cmd$ ");

    cmdLineStart = currCursor.position();
    cmdLineEnd   = cmdLineStart;

    setTextCursor(currCursor);

    // determine the vertical scrollbar of the QTextEdit element
    QScrollBar *vsb = this->verticalScrollBar();

    // get the maximum position of the slider and then setup slider's position to the maximum
    // maximum position = length of the document - length of the page
    int sliderMax = vsb->maximum();

    vsb->setSliderPosition(sliderMax);

    return;
}

/**
 * @brief TerminalWindow::commandReadyRead
 *   Is called if there is a command output available in the process.
 * @param channel
 */
void TerminalWindow::commandReadyRead(int channel)
{
    QString    strData;
    QByteArray buffData;
    int        buffSize;

  #ifdef DEBUG
    cout << "Command output read.\n";
    cout.flush();
  #endif

    currCursor = textCursor();

    process->setCurrentReadChannel(channel);

    // returns an installed QTextCodec for the IBM 850 encoding; Windows-1252 not usable due to the umlauts
    // in the character set Windows-1252 are on another position than the umlauts in the ASCII character set
    QTextCodec *codec = QTextCodec::codecForName("IBM 850");

    // auto=alias for QStringDecoder; Latin1 not usable due to the character set contains control characters
    // in the range from 80h to 9Fh
    //QStringDecoder/*auto*/ toUtf16 = QStringDecoder(QStringDecoder::Latin1);

    do {
        buffData = process->read(64);
        buffSize = buffData.size();

        // Exists the needed Codec?
        if (codec != nullptr && wrkDrive != '/') {
            strData = codec->toUnicode(buffData);
          //QString strData = toUtf16.decode(buffData)/*toUtf16(buffData)*/;
        } else {
            strData = QString(buffData);
        }

        currCursor.insertText(strData);

    } while (buffSize >= 64);

    setTextCursor(currCursor);

    return;
}

/**
 * @brief TerminalWindow::commandFinished
 *   Is called when the process is finished. Then the process is in the not running state.
 * @param exitCode
 * @param exitStatus
 */
void TerminalWindow::commandFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  #ifdef DEBUG
    cout << "Command executing finished.\n";
    cout.flush();
  #endif

    currCursor = textCursor();

    currCursor.insertText("cmd$ ");
    cmdLineStart = currCursor.position();
    cmdLineEnd   = cmdLineStart;

    setTextCursor(currCursor);

    // determine the vertical scrollbar of the QTextEdit element
    QScrollBar *vsb = this->verticalScrollBar();

    // get the maximum position of the slider and then setup slider's position to the maximum
    // maximum position = length of the document - length of the page
    int sliderMax = vsb->maximum();

    vsb->setSliderPosition(sliderMax);

    return;
}
