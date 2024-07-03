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

#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <list>
#include <string>
#include <QBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>

using namespace std;

class AddDialog : public QDialog
{
    Q_OBJECT

    QLineEdit *command;
    QLineEdit *category;
    QListView *categories;
    QStandardItemModel *catModel;

  private slots:
    void setCategory(const QModelIndex &index);
    void addCommand();

  public:
    AddDialog(QString *lastCommand, list<string> catList, QWidget *parent);

    QString getCommand() { return command->text(); }
    QString getCategory() { return category->text(); }
};

#endif // ADDDIALOG_H
