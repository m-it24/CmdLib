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

#ifndef TERMINALWINDOW_H
#define TERMINALWINDOW_H

#include <iostream>
#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QFont>
#include <QKeyEvent>
#include <QProcess>
#include <QScrollBar>
#include <QString>
#include <QTextCodec>
#include <QTextCursor>
#include <QTextEdit>
#include "main.h"

class TerminalWindow : public QTextEdit
{
    Q_OBJECT

    int cmdLineStart;
    int cmdLineEnd;

    QProcess *process;
    QDir     *workDir;
    QString   homeDir;
    QChar     wrkDrive;
    QString  *cmdEntered;

    QTextCursor currCursor;

    void keyPressEvent(QKeyEvent *event);

    QStringList *getCommandParts(QString *cmd);

  private slots:
    void commandExternal(QStringList *cmdParts);
    void commandStarted();
    void commandError(QProcess::ProcessError error);
    void commandReadyRead(int channel);
    void commandFinished(int exitCode, QProcess::ExitStatus exitStatus);

  public:
    enum BuiltInCmds { CD, CLEAR, EXIT, NONE };

    explicit TerminalWindow(QWidget *parent = nullptr);

  public slots:
    void commandInternal(TerminalWindow::BuiltInCmds cmd, QStringList *cmdParts);
    void setCommandSelected(QString *cmd);

  signals:
    void commandInt_signal(TerminalWindow::BuiltInCmds,QStringList *);
    void commandExt_signal(QStringList *);
    void commandEntered_signal(QString *);
};

#endif // TERMINALWINDOW_H
