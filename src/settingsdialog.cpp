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

#include "settingsdialog.h"

// initialisation of static attributes

// language setups
const SettingsDialog::_langList SettingsDialog::langList[NUMLANGUAGES] =
    { { tr("German").toStdString() , "de", QLocale::German  },    // German
      { tr("English").toStdString(), "en", QLocale::English } };  // English

QTranslator SettingsDialog::translator;

bool SettingsDialog::transSet = false;

/**
 * @brief SettingsDialog::SettingsDialog
 */
SettingsDialog::SettingsDialog(Settings *stgs, QWidget *parent = nullptr) : QDialog(parent)
{
    // instead of calling the parent constructor in the element initialisation
    // the parent constructor can called in the derived class

    // different super class construcotr calls
    //QDialog((QDialog *)parent);
    //QDialog(parent,Qt::Widget);

  // inherit the configurations
    settings      = stgs;
    settingsLast  = new Settings;
    *settingsLast = *stgs;

  // setup the dialog attributes
    setModal(true);
    //setWindowTitle(tr("Settings"));
    setFixedSize(QSize(350,250));

  // create the dialog's layout
    QBoxLayout *settingsLayout = new QBoxLayout(QBoxLayout::TopToBottom,this);

  // language configuration

    // headline
    lblLangTitle = new QLabel();

    settingsLayout->addWidget(lblLangTitle,0,Qt::AlignLeft);

    // display the available languages
    langModel   = new QStandardItemModel();
    lvLanguages = new QListView();

    lvLanguages->setModel(langModel);
    lvLanguages->setEditTriggers(QAbstractItemView::NoEditTriggers);

    settingsLayout->addWidget(lvLanguages);

    // display the choosen language
    lblLangChoosen = new QLabel();

    settingsLayout->addWidget(lblLangChoosen);

  // create a line for Ok/Apply/Cancel
    QGridLayout *confirmLayout = new QGridLayout();

    btnOk = new QPushButton();
    confirmLayout->addWidget(btnOk,0,0);

    connect(btnOk,SIGNAL(clicked(bool)),this,SLOT(accept()));
    //connect(btnOk,&QPushButton::clicked,this,&SettingsDialog::accept);

    btnApply = new QPushButton();
    confirmLayout->addWidget(btnApply,0,1);

    connect(btnApply,SIGNAL(clicked(bool)),this,SLOT(apply()));

    btnCancel = new QPushButton();
    confirmLayout->addWidget(btnCancel,0,2);

    connect(btnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));

    settingsLayout->addLayout(confirmLayout,0);

    translateDialog();

    this->setLayout(settingsLayout);

    return;
}

/**
 * @brief SettingsDialog::langInit
 * @param lang
 */
void SettingsDialog::langInit(string lang)
{
    // Is a translator already installed?
    if (transSet) {
        QCoreApplication::qApp->removeTranslator(&translator);
        transSet = false;
      #ifdef DEBUG
        cout << "Translator removed.\n";
      #endif
    }

    // install the translation for the choosen language
    for (int idx = 0; idx < NUMLANGUAGES; idx++) {
        if (langList[idx].locId == lang) {
            if (translator.load(langList[idx].locCode,"loc/cmdlib","_")) {
                QApplication *app = (QApplication *)QCoreApplication::instance();  // determine the current instance of QCoreApplication
                //if (QCoreApplication::qApp->installTranslator(&translator)) {  // qApp = macro for determining the QCoreApplication instance
                if (((QCoreApplication *)app)->installTranslator(&translator)) {
                  #ifdef DEBUG
                    cout << "Translator installed.\n";
                  #endif
                    transSet = true;
                } else {
                  #ifdef DEBUG
                    cout << "ERROR! Translator not installed.\n";
                  #endif
                }
            } else {
              #ifdef DEBUG
                cout << "ERROR! Translation not loaded.\n";
              #endif
            }
        }
    }

    cout << flush;
    return;
}

/**
 * @brief SettingsDialog::changeEvent
 * @param event
 */
void SettingsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateDialog();
    } else {
        /*this->*/QDialog::changeEvent(event);  // not handled events are passed to the super class
    }

    return;
}

/**
 * @brief SettingsDialog::setupLanguage
 */
void SettingsDialog::setupLanguage()
{
    QModelIndex currIdx = lvLanguages->currentIndex();

    int currNum = currIdx.row();

    settings->language = langList[currNum].locId;

    langInit(settings->language);

    return;
}

/**
 * @brief retranslateDialog
 */
void SettingsDialog::translateDialog()
{
    int idx;

  // display the window title
    setWindowTitle(tr("Settings"));

  // language setting section
    lblLangTitle->setText(tr("Language"));

  // display the available languages
    langModel->clear();  // clear the model data

    for (int cntr = 0; cntr < NUMLANGUAGES; cntr++) {
        QStandardItem *item = new QStandardItem(tr(langList[cntr].lang.c_str()));
        langModel->appendRow(item);
    }

  // display the choosen language
    for (idx = 0; idx < NUMLANGUAGES; idx++) {
        if (langList[idx].locId == settings->language) {
            lblLangChoosen->setText(tr("Choosen Language")+": "+tr(langList[idx].lang.c_str()));
            break;
        }
    }

    QItemSelectionModel *ism = lvLanguages->selectionModel();  // get the model for the current selection of the list view

    QStandardItem *si = langModel->item(idx);  // determine the choosen item

    QModelIndex mi = langModel->indexFromItem(si);  // get the model index of the choosen item
    //QModelIndex mi = langModel->index(0,0,QModelIndex());  // get the model index by specifying the position

    ism->setCurrentIndex(mi,QItemSelectionModel::Clear | QItemSelectionModel::Select);  // display the choosen entry
    //lvLanguages->setCurrentIndex(mi);

  // display the button labeling
    btnOk->setText(tr("Ok"));
    btnApply->setText(tr("Apply"));
    btnCancel->setText(tr("Cancel"));

    return;
}

/**
 * @brief SettingsDialog::accept
 */
void SettingsDialog::accept()
{
    setupLanguage();

    QDialog::accept();

    delete settingsLast;
    return;
}

/**
 * @brief SettingsDialog::apply
 */
void SettingsDialog::apply()
{
    setupLanguage();
    return;
}

/**
 * @brief SettingsDialog::reject
 *   Exits the dialog by pressing the Cancel button, the exit cross on top right or the ESC key.
 */
void SettingsDialog::reject()
{
    langInit(settingsLast->language);

    QDialog::reject();

    delete settingsLast;
    return;
}
