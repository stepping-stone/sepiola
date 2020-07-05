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

#ifndef ABSTRACT_INFORMING_PROCESS_HH
#define ABSTRACT_INFORMING_PROCESS_HH

#include <QDateTime>
#include <QMetaType>
#include <QObject>
#include <QString>

#include "utils/const_utils.hh"
#include "utils/string_utils.hh"

/**
 * The AbstractInformingProcess class provides methods for emitting message signals
 * @author Bruno Santschi, santschi@puzzle.ch
 */

class AbstractInformingProcess : public QObject
{
    Q_OBJECT

public:
    /**
     * Destroys the AbstractInformingProcess
     */
    virtual ~AbstractInformingProcess();

signals:
    void infoSignal(const QString &text);
    void errorSignal(const QString &text);
    void progressSignal(const QString &taskText,
                        float percentFinished,
                        const QDateTime &timeRemaining,
                        StringPairList infos = StringPairList());
    void finalStatusSignal(ConstUtils::StatusEnum status);
};

inline AbstractInformingProcess::~AbstractInformingProcess() {}

#endif /* ABSTRACT_INFORMING_PROCESS_HH */
