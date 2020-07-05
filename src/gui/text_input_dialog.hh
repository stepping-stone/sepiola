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

#ifndef TEXT_INPUT_DIALOG_HH
#define TEXT_INPUT_DIALOG_HH

#include <QDialog>
#include <QString>

#include "ui_text_input_dialog.h"

/**
 * The TextInputDialog class provides a dialog for inputing a text
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class TextInputDialog : public QDialog, private Ui::TextInputDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a TextInputDialog
     */
    TextInputDialog(const QString &title,
                    const QString &label,
                    const QString &text = "",
                    const int &position = 0);

    /**
     * Destroys the TextInputDialog
     */
    virtual ~TextInputDialog();

signals:
    void textEntered(const QString &text);
    void textEdited(const int &position, const QString &text);

private slots:
    void accept();
    void reject();

private:
    QString text;
    int position;
};

#endif
