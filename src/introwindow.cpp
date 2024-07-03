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

#include "introwindow.h"

/**
 * @brief IntroWindow::IntroWindow
 *   Creates the introduction window.
 * @param lang  language
 * @param parent
 * @param f
 */
IntroWindow::IntroWindow(string lang, QWidget *parent, Qt::WindowFlags f) : QWidget(parent,f)
{
    QVBoxLayout *introLayout = new QVBoxLayout();
    QTextEdit   *introDoc    = new QTextEdit();
    QPushButton *buttonOk    = new QPushButton(tr("Ok"));

    connect(buttonOk,SIGNAL(clicked(bool)),this,SLOT(exitIntroduction()));

  // create the introduction
    QTextCursor cursor = introDoc->textCursor();

  // headline
    QFont *introFont = new QFont("DejaVu Sans",  // font family
                                14,      // point size
                                -1,      // weight
                                false);  // italic

    introFont->setUnderline(true);

    QTextCharFormat *charFormat = new QTextCharFormat();

    charFormat->setFont(*introFont);

    cursor.setCharFormat(*charFormat);
    cursor.insertText("\n");
    cursor.insertText(tr("Introduction to Command Library"));
    cursor.insertText("\n\n");

  // preamble
    introFont->setPointSize(12);
    introFont->setUnderline(false);

    charFormat->setFont(*introFont);

    cursor.setCharFormat(*charFormat);
    cursor.insertText(tr("Command library is used to add commands entered on the command line to a library."
                         " Then you can call commands from the library if you need them."));
    cursor.insertText("\n\n\n");

  // picture
    if (lang == "de") {
        cursor.insertImage(QImage("res/introduction_de.png"));
    } else {
        cursor.insertImage(QImage("res/introduction_en.png"));
    }
    cursor.insertText("\n\n");

  // description
    introFont->setPointSize(12);

    charFormat->setFont(*introFont);

    cursor.setCharFormat(*charFormat);
    cursor.insertText(tr("At the prompt you can enter a command. Like in a terminal window press enter to execute"
                         " the command. Then the entered command is displayed on the last command line. To add the"
                         " command to the library press the 'Add' button in the command panel. Then a window opens"
                         " where you can add your executed command to a category in the library. You can also add"
                         " commands on the last command line to the library, without executing them."
                         " To call a command from the library choose the command from the category/command choice."
                         " Then the command is displayed at the prompt. Now you can execute or modify that command"
                         " or delete it from the library."));

    cursor.setPosition(0);

    introDoc->setReadOnly(true);
    introDoc->setTextCursor(cursor);

  // place the elements into the layout
    introLayout->addWidget(introDoc);
    introLayout->addWidget(buttonOk);

  // if the main window passed, get the position and size from the main window
    int x=0,y=0,width=0,height=0;

    if (parent != nullptr) {
        QRect rectMainWin = parent->geometry();
        rectMainWin.getRect(&x,&y,&width,&height);
    }

  // show the introduction window relative to the main window
    setAttribute(Qt::WA_DeleteOnClose);  // if the method close() is called, the widget is removed
    setWindowTitle(tr("Introduction"));
    setGeometry(x+100,y+50,850,600);
    setLayout(introLayout);
    show();

    return;
}

/**
 * @brief IntroWindow::~IntroWindow
 */
IntroWindow::~IntroWindow()
{
    return;
}

/**
 * @brief MainWindow::exitIntroduction
 */
void IntroWindow::exitIntroduction()
{
    //hide();  // not required due to close() closes the window

    close();

    // It is not necessary to delete all child elements due to removing
    // the introduction window removes all child elements automatically.

    //delete this;  // it is not possible here because the object is removed before leaving the method

    return;
}
