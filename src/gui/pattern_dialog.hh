/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008  stepping stone GmbH
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

#ifndef PATTERN_DIALOG_HH
#define PATTERN_DIALOG_HH

#include <QDialog>

#include "ui_pattern_dialog.h"

/**
 * The PatternDialog class provides a dialog for editing include or exclude patterns
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class PatternDialog : public QDialog, private Ui::PatternDialog
{
	Q_OBJECT
	
public:
	/**
	 * Constructs a PatternDialog
	 */
	PatternDialog( const QString& title, const QString& label, const QStringList& patternList );
	
	/**
	 * Destroys the PatternDialog
	 */
	virtual ~PatternDialog();

signals:
	void getPatternList( const QStringList& patternList );

private slots:
	void accept();
	void reject();
	void on_btnAdd_pressed();
	void on_btnEdit_pressed();
	void on_btnRemove_pressed();
	void addPattern( const QString& pattern );
	void editPattern( const int& position, const QString& text );

private:
	QStringList patternList;	
};

#endif
