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

#ifndef ABSTRACT_SCHEDULER_HH
#define ABSTRACT_SCHEDULER_HH

#include <QObject>
#include <QString>
#include <QTime>

#include "tools/abstract_informing_process.hh"

/**
 * The AbstractScheduler class provides methods for using a scheduler
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class AbstractScheduler : public AbstractInformingProcess
{
    Q_OBJECT

public:
    /**
     * Destroys the AbstractScheduler
     */
    virtual ~AbstractScheduler();

    /**
     * Checks whether the scheduler supports running the task after booting or not
     * @return flag whether scheduling on start is supported
     */
    virtual bool isSchedulingOnStartSupported() = 0;

    /**
     * Schedules the given task at the given time for the given days
     * @param execName the executable's name
     * @param cliArgument argument for starting in command line mode
     * @param time time to start the task
     * @param days a boolean array indicating the days to start the task
     */
    virtual void scheduleTask(const QString &execName,
                              const QString &cliArgument,
                              const QTime &time,
                              const bool days[])
        = 0;

    /**
     * Schedules the given task to start after booting
     * @param execName the executable's name
     * @param cliArgument argument for starting in command line mode
     * @param minutesToDelay
     */
    virtual void scheduleTask(const QString &execName,
                              const QString &cliArgument,
                              const int &minutesToDelay)
        = 0;

    /**
     * Updates possible existing backup job entry
     * @param execName the executable's name
     * @param cliArgument argument for starting in command line mode
     */
    virtual bool updateExistingTask(const QString &execName, const QString &cliArgument) = 0;

    /**
     * Deletes possible existing backup job entry
     * @param cliArgument argument for starting in command line mode
     */
    virtual void deleteExistingTask(const QString &execName, const QString &cliArgument) = 0;

signals:
    void askForPassword(const QString &username,
                        bool isUsernameEditable,
                        int *result = 0,
                        const QString &msg = "");
};

inline AbstractScheduler::~AbstractScheduler() {}

#endif
