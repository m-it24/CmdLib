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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <iostream>
#include <string>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QDialog>
#include <QItemSelectionModel>
#include <QLabel>
#include <QListView>
#include <QModelIndex>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTranslator>
#include "main.h"

using namespace std;

class SettingsDialog : public QDialog
{
    Q_OBJECT

    #define NUMLANGUAGES 2

    struct _langList {
        string  lang;
        string  locId;
        QLocale locCode;
    };

    static const _langList langList[NUMLANGUAGES];
    static QTranslator translator;
    static bool transSet;

    QLabel      *lblLangTitle;
    QLabel      *lblLangChoosen;
    QListView   *lvLanguages;
    QPushButton *btnOk;
    QPushButton *btnApply;
    QPushButton *btnCancel;

    QStandardItemModel *langModel;

    string langChoosen;

    void setupLanguage();
    void translateDialog();
    void changeEvent(QEvent *);

  private slots:
    void accept();
    void apply();
    void reject();

  public:
    struct Settings {
        string language;
    };

    Settings *settings;
    Settings *settingsLast;

    SettingsDialog(Settings *,QWidget *);

    static void langInit(string);
};

#endif // SETTINGSDIALOG_H
