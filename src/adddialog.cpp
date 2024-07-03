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

#include "adddialog.h"

/**
 * @brief AddDialog::AddDialog
 * @param lastCommand
 * @param catList
 * @param parent
 */
AddDialog::AddDialog(QString *lastCommand, list<string> catList, QWidget *parent = nullptr) : QDialog(parent)
{
    // instead of calling the parent constructor in the element initialisation
    // the parent constructor can called in the derived class

    //QDialog(parent,Qt::Widget);

  // set the dialog attributes
    setModal(true);
    setWindowTitle(tr("Add Command"));
    setFixedSize(QSize(350,250));

  // create the layout for the dialog
    QBoxLayout *addLayout = new QBoxLayout(QBoxLayout::TopToBottom,this);

  // title for the category line
    addLayout->addWidget(new QLabel(tr("Category")+":"),0,Qt::AlignLeft);

  // category input line
    category = new QLineEdit(this);

    addLayout->addWidget(category);

  // show the available categories
    catModel = new QStandardItemModel();

    list<string>::iterator catIter = catList.begin();

    int catSize = catList.size();

    for (int cntr = 0; cntr < catSize; cntr++) {
        QStandardItem *item = new QStandardItem(catIter->c_str());
        catModel->appendRow(item);
        catIter++;
    }

    categories = new QListView();

    categories->setModel(catModel);
    categories->setEditTriggers(QAbstractItemView::NoEditTriggers);

    addLayout->addWidget(categories);

    connect(categories,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(setCategory(const QModelIndex &)));

  // title for the command line
    addLayout->addWidget(new QLabel(tr("Command")+":"),0,Qt::AlignLeft);

  // command input line
    string lc = lastCommand->toStdString();

    command = new QLineEdit(lc.c_str(),this);

    addLayout->addWidget(command);

  // add the Add/Cancel buttons
    QGridLayout *confirmLayout = new QGridLayout();

    QPushButton *addButton = new QPushButton(tr("Add"));
    confirmLayout->addWidget(addButton,0,0);

    connect(addButton,SIGNAL(clicked(bool)),this,SLOT(addCommand()));

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    confirmLayout->addWidget(cancelButton,0,1);

    connect(cancelButton,SIGNAL(clicked(bool)),this,SLOT(reject()));

    addLayout->addLayout(confirmLayout,0);

    this->setLayout(addLayout);

    return;
}

/**
 * @brief AddDialog::setCategory
 * @param index
 */
void AddDialog::setCategory(const QModelIndex &index)
{
    QStandardItem *item = catModel->itemFromIndex(index);

    QString cat = item->text();

    category->clear();
    category->setText(cat);

    return;
}

/**
 * @brief AddDialog::addCommand
 */
void AddDialog::addCommand()
{
    // Leaving the adding dialog is only possible
    // if a category was choosen or entered.
    if (category->text().size() > 0) {
        accept();
    }
    return;
}
