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

#include "gui/text_input_dialog.hh"

TextInputDialog::TextInputDialog(const QString &title,
                                 const QString &label,
                                 const QString &text,
                                 const int &position)
{
    setupUi(this);
    setWindowTitle(title);
    this->labelTitle->setText(label);
    this->lineEditText->setText(text);
    this->position = position;
}

TextInputDialog::~TextInputDialog() {}

void TextInputDialog::accept()
{
    emit textEntered(this->lineEditText->text());
    emit textEdited(this->position, this->lineEditText->text());
    this->done(0);
}

void TextInputDialog::reject()
{
    this->done(0);
}
