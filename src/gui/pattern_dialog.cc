/*
#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
#|
#| This program is free software; you can redistribute it and/or
#| modify it under the terms of the GNU General Public License
#| Version 2 as published by the Free Software Foundation.
#|
#| This program is distributed in the hope that it will be useful,
#| but WITHOUT ANY WARRANTY; without even the implied warranty of
#| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#| GNU General Public License for more details.
#|
#| You should have received a copy of the GNU General Public License
#| along with this program; if not, write to the Free Software
#| Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QDebug>

#include "gui/pattern_dialog.hh"
#include "gui/text_input_dialog.hh"
#include "settings/settings.hh"

PatternDialog::PatternDialog(const QString &title,
                             const QString &label,
                             const QStringList &patternList)
{
    setupUi(this);
    setWindowTitle(title);
    this->labelTitle->setText(label);

    this->listWidgetPattern->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->patternList = patternList;

    foreach (QString pattern, patternList) {
        new QListWidgetItem(pattern, this->listWidgetPattern);
    }
}

PatternDialog::~PatternDialog() {}

void PatternDialog::accept()
{
    emit getPatternList(this->patternList);
    this->done(0);
}

void PatternDialog::reject()
{
    this->done(0);
}

void PatternDialog::addPattern(const QString &pattern)
{
    new QListWidgetItem(pattern, this->listWidgetPattern);
    this->patternList.append(pattern);
}

void PatternDialog::editPattern(const int &position, const QString &text)
{
    this->patternList.replace(position, text);
    QListWidgetItem *item = this->listWidgetPattern->item(position);
    item->setText(text);
}

void PatternDialog::on_btnAdd_clicked()
{
    TextInputDialog inputDialog(tr("Add"), tr("Add a pattern"));
    QObject::connect(&inputDialog, SIGNAL(textEntered(QString)), this, SLOT(addPattern(QString)));
    inputDialog.exec();
    QObject::disconnect(&inputDialog, SIGNAL(textEntered(QString)), this, SLOT(addPattern(QString)));
}

void PatternDialog::on_btnEdit_clicked()
{
    foreach (QListWidgetItem *selectedItem, this->listWidgetPattern->selectedItems()) {
        QString patternText = selectedItem->text();
        int position = this->listWidgetPattern->row(selectedItem);
        TextInputDialog inputDialog(tr("Edit"),
                                    tr("Edit %1 pattern").arg(patternText),
                                    patternText,
                                    position);
        QObject::connect(&inputDialog,
                         SIGNAL(textEdited(int, QString)),
                         this,
                         SLOT(editPattern(int, QString)));
        inputDialog.exec();
        QObject::disconnect(&inputDialog,
                            SIGNAL(textEdited(int, QString)),
                            this,
                            SLOT(editPattern(int, QString)));
    }
}

void PatternDialog::on_btnRemove_clicked()
{
    foreach (QListWidgetItem *selectedItem, this->listWidgetPattern->selectedItems()) {
        int position = this->listWidgetPattern->row(selectedItem);
        this->patternList.takeAt(position);
        delete this->listWidgetPattern->takeItem(position);
    }
}
