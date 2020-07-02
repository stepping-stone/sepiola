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
#include <QLocale>
#include <QStringList>

#include "cli/cli_manager.hh"
#include "settings/settings.hh"
#include "test/test_manager.hh"
#include "tools/at.hh"

const QString At::AT_NAME = "at";

At::At() {}

At::~At() {}

void At::scheduleTask(const QString &execName,
                      const QString &cliArgument,
                      const QTime &time,
                      const bool days[])
{
    qDebug() << "At::scheduleTask( " << execName << ", " << cliArgument << ", " << time
             << ", days )";

    // delete old at job
    this->deleteExistingTask(execName, cliArgument);

    // add new backup job
    int hour = time.hour();
    int minute = time.minute();

    QLocale locale = getAtBinaryLocale();
    QStringList weekdayNames = getWeekdayNames(locale);
    QString dayString;
    for (int i = 0; i < 7; i++) {
        if (days[i]) {
            if (dayString != "") {
                dayString.append(",");
            }
            dayString.append(weekdayNames.at(i));
        }
    }

    QStringList arguments;
    arguments << QVariant(hour).toString() + ":" + QVariant(minute).toString();
    arguments << "/every:" + dayString;
    arguments << execName;
    arguments << cliArgument;

    qDebug() << "Creating job " << AT_NAME << arguments;
    createProcess(AT_NAME, arguments);
    start();
    waitForFinished();

    QString lineData;
    lineData = readAllStandardError();
    if (lineData != "") {
        emit errorSignal(lineData);
    } else {
        emit infoSignal(QObject::tr("Scheduled task has been created"));
    }
}

bool At::updateExistingTask(const QString &execName, const QString &cliArgument)
{
    // ignoring the passed cliArgument, updating AT based on settings->getApplicationName()
    qDebug() << "At::updateExistingTask( " << execName << ", " << cliArgument << " )";

    createProcess(AT_NAME);
    start();

    QStringList existingEntries;
    QString line;
    while (blockingReadLine(&line)) {
        existingEntries << line;
    }

    // find out if there are entries (this would be the third line)
    if (existingEntries.size() > 2) {
        for (int i = 2; i < existingEntries.size(); i++) {
            // find possible existing backup job entry
            QString entry = existingEntries.at(i);
            if (entry.contains(Settings::getInstance()->getThisApplicationExecutable())) {
                QTextStream stream(&entry, QIODevice::ReadOnly);
                uint id = 0;
                stream >> id;
                if (id == 0) {
                    // status text is at first position, so get the second value
                    QString temp;
                    stream >> temp;
                    stream >> id;
                }
                qDebug() << "Reading at-job information for id " << id;
                QStringList arguments;
                arguments << QString::number(id);
                createProcess(AT_NAME, arguments);
                start();

                QStringList propertyEntries;
                while (blockingReadLine(&line)) {
                    propertyEntries << line;
                }

                /*
                 Task ID:       5
                 Status:        OK
                 Schedule:      Each M T W Th
                 Time of day:   22:00 PM
                 Interactive:   No
                 Command:       readme.txt
                 */
                QString days_entry;
                QString time_entry;
                for (int i = 0; i < propertyEntries.size(); i++) {
                    if (propertyEntries.at(i).startsWith("Schedule:")) {
                        QString prop_entry = propertyEntries.at(i);
                        days_entry = prop_entry.mid(prop_entry.indexOf("Each ") + 5)
                                         .trimmed()
                                         .replace(" ", ",");
                    }
                    if (propertyEntries.at(i).startsWith("Time of day:")) {
                        QString prop_entry = propertyEntries.at(i);
                        time_entry = prop_entry.mid(prop_entry.indexOf("Time of day") + 12)
                                         .trimmed()
                                         .replace(" ", "");
                    }
                }
                // AT outputs "22:00 PM" on some systems, but doesn't accept "22:00PM" as input,
                // delete "PM" in cases where hour >= 12
                if (time_entry.left(time_entry.indexOf(":")).toInt() >= 12) {
                    time_entry = time_entry.left(5);
                }
                // Deleting at-job
                qDebug() << "Deleting at-job with id " << id;
                arguments.clear();
                arguments << QString::number(id);
                arguments << "/delete";
                createProcess(AT_NAME, arguments);
                start();
                waitForFinished();

                qDebug() << "Recreating at-job";
                arguments.clear();
                arguments << time_entry;
                arguments << "/every:" + days_entry;
                arguments << execName;
                arguments << cliArgument;

                qDebug() << "Creating job " << AT_NAME << arguments;
                createProcess(AT_NAME, arguments);
                start();
                waitForFinished();
            }
        }
        return true;
    } else {
        return false;
    }
}

void At::deleteExistingTask(const QString & /*execName*/, const QString &cliArgument)
{
    // ignoring the passed cliArgument, deleting AT based on settings->getApplicationName()
    // read at entries
    createProcess(AT_NAME);
    start();

    QStringList existingEntries;
    QString line;
    while (blockingReadLine(&line)) {
        existingEntries << line;
    }

    // find out if there are entries (this would be the third line)
    if (existingEntries.size() > 2) {
        for (int i = 2; i < existingEntries.size(); i++) {
            // find possible existing backup job entry
            QString entry = existingEntries.at(i);
            if (entry.endsWith(cliArgument + "\n")) {
                // remove existing backup job entry
                QTextStream stream(&entry, QIODevice::ReadOnly);
                uint id = 0;
                stream >> id;
                if (id == 0) {
                    // status text is at first position, so get the second value
                    QString temp;
                    stream >> temp;
                    stream >> id;
                }
                qDebug() << "Deleting at-job " << id;
                QStringList arguments;
                arguments << QString::number(id);
                arguments << "/delete";

                createProcess(AT_NAME, arguments);
                start();
                waitForFinished();
            }
        }
    }
}

QStringList At::getWeekdayNames(const QLocale &locale)
{
    QStringList result;
    switch (locale.language()) {
    case QLocale::German:
        result << "Mo"
               << "Di"
               << "Mi"
               << "Do"
               << "Fr"
               << "Sa"
               << "So";
        return result;
    default:
        break;
    }
    result << "M"
           << "T"
           << "W"
           << "Th"
           << "F"
           << "S"
           << "Su";
    return result;
}

QLocale At::getAtBinaryLocale()
{
    qDebug() << "At::getAtBinaryLocale()";

    QStringList arguments;
    arguments << "/?";
    createProcess(AT_NAME, arguments);
    start();

    waitForReadyRead();
    QString lineData;
    blockingReadLine(&lineData);
    qDebug() << "at line: " << lineData;
    if (lineData.startsWith("Mit dem Befehl AT")) {
        return QLocale(QLocale::German);
    }
    return QLocale(QLocale::English);
}

bool At::isSchedulingOnStartSupported()
{
    return false;
}

void At::testAtScheduleTask()
{
    At at;
    QTime time(15, 51);
    bool days[7] = {true, false, true, false, true, true, false};
    at.scheduleTask("C:/ssbackup/debug/src/sepiola.exe", CliManager::SCHEDULE_ARGUMENT, time, days);
}

namespace {
int dummy1 = TestManager::registerTest("testAtScheduleTask", At::testAtScheduleTask);
}
