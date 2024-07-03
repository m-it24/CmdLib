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

#ifndef INTROWINDOW_H
#define INTROWINDOW_H

#include <string>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

using namespace std;

class IntroWindow : public QWidget
{
    Q_OBJECT

  private slots:
    void exitIntroduction();

  public:
    explicit IntroWindow(string lang, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~IntroWindow();
};

#endif // INTROWINDOW_H
