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

#include <QApplication>
#include <QByteArray>
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QLocale>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "exception/login_exception.hh"
#include "model/restore_name.hh"
#include "settings/platform.hh"
#include "settings/settings.hh"
#include "test/test_manager.hh"
#include "tools/plink.hh"
#include "utils/host_file_utils.hh"

const QString Plink::PUTTY_HEADER_FIRST_LINE = "PuTTY-User-Key-File";
const QString Plink::OPEN_SSH_HEADER_FIRST_LINE = "-----BEGIN DSA PRIVATE KEY-----";
const QString Plink::LOGIN_ECHO_MESSAGE = "LoggedIn";
const QString Plink::START_PRIVATE_KEY_ECHO_MESSAGE = "StartPrivateKey";
const QString Plink::END_PRIVATE_KEY_ECHO_MESSAGE = "EndPrivateKey";
const QString Plink::START_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE = "StartPrivateOpenSshKey";
const QString Plink::END_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE = "EndPrivateOpenSshKey";
const QString Plink::KEY_LENGTH = "2048";
const QString Plink::KEY_TYPE = "dsa";

Plink::Plink()
    : plinkName(Settings::getInstance()->getPlinkName())
    , scriptName(Settings::getInstance()->getScriptName())
{}

Plink::~Plink() {}

bool Plink::loginWithKey()
{
    qDebug() << "Plink::loginWithKey()";
    Settings *settings = Settings::getInstance();

    QString privatePuttyKeyFileName = settings->createPrivatePuttyKeyFile();
    QString privateOpenSshKeyFileName = settings->createPrivateOpenSshKeyFile();

    if (privatePuttyKeyFileName == "" || privateOpenSshKeyFileName == "") {
        return false; // force generation of keys
    }

    HostFileUtils::addPuttyKeyToOpenSshKeyFile(settings->getServerName(),
                                               QDir::home().absolutePath() + "/.putty/sshhostkeys",
                                               settings->getSshConfigDataDir() + "known_hosts");

    QStringList arguments;
    arguments << "-noagent";
    arguments << "-i" << privatePuttyKeyFileName;
    /* $$$ TODO [dt] check if this is correct */ // arguments << "-o" << "IdentitiesOnly=yes"; //
                                                 // doesn't work lonely so
    arguments << settings->getServerUserName() + "@" + settings->getServerName();
    arguments << "sh -c " + StringUtils::quoteText("echo " + LOGIN_ECHO_MESSAGE, "'");
    createProcess(this->plinkName, arguments);
    setProcessChannelMode(QProcess::MergedChannels);
    start();
    waitForReadyRead();
    QString line = readAll();
    line.replace("\n", "");
    line.replace("\r", "");
    if (line != LOGIN_ECHO_MESSAGE) {
        // login failed if we have an output
        qWarning() << "Login failed. Unexpected line is: " << line;
        terminate();
        waitForFinished();
        return false;
    } else {
        waitForFinished();
        return (exitCode() == 0);
    }
}

bool Plink::assertCorrectFingerprint()
{
    Settings *settings = Settings::getInstance();
    return assertCorrectFingerprint(settings->getServerUserName(),
                                    settings->getServerName(),
                                    settings->getServerKey());
}

bool Plink::assertCorrectFingerprint(const QString &userName,
                                     const QString &serverName,
                                     const QString &savedKey)
{
    qDebug() << "Plink::assertCorrectFingerprint(userName=" << userName
             << ", serverName=" << serverName << ", savedKey=" << savedKey << ")";
    QStringList arguments(
        {"-noagent", userName + "@" + serverName, StringUtils::quoteText("sh -c ':'", "\"")});

// Workaround for Linux since password prompt does not appear on stdout
#ifndef Q_OS_WIN32
    QStringList plinkCommand;
    plinkCommand << this->plinkName;
    plinkCommand << arguments;
    QStringList scriptArguments(
        {"--quiet", "--return", "--log-out", "/dev/null", "--command", plinkCommand.join(" ")});
#endif

#ifdef Q_OS_WIN32
    createProcess(this->plinkName, arguments);
#else
    createProcess(this->scriptName, scriptArguments);
#endif
    setProcessChannelMode(QProcess::MergedChannels);
    start();

    if (!waitForReadyRead()) {
        if (!waitForFinished()) {
            qDebug() << "Timeout occurred while waiting for plink output";
            terminate();
            return false;
        }
        // for some reason, putty has the key already in its cache
        if ((exitStatus() == QProcess::NormalExit) && (exitCode() == 0))
            return true;

        qDebug() << "Unknown error occurred while waiting for plink output";
        return false;
    }

    // if the key has already been cached, the first line is either
    //  "username@host password:" or
    //  "Using keyboard-interactive authentication."
    QString line = readLine();
    if (line.contains('@') || line.startsWith("Using")
        || line.startsWith("login")) // password prompt
    {
        // key has already been cached
        qDebug() << "Key has already been cached";
        terminate();
        return true;
    }
    if (line.startsWith("Unable to open connection")) {
        emit errorSignal(QObject::tr("Unable to open connection"));
        terminate();
        return false;
    }
    if (line.startsWith("FATAL ERROR")) {
        qDebug() << "Key has already been cached but connection failed (probably wrong username)";
        terminate();
        return true;
    }
    while (!line.startsWith("ssh-rsa")) {
        blockingReadLine(&line);
    }
    QStringList splittedLine = line.split(" ");
    if (splittedLine.size() != 3) {
        qCritical() << "Fingerprint output format not recognized";
        terminate();
        return false;
    }
    QString serverKey = splittedLine.at(2);
    serverKey = serverKey.left(serverKey.lastIndexOf(":") + 3);
    if (serverKey.compare(savedKey) == 0) {
        qDebug() << "Correct server key";
        write("y");
        write(Platform::EOL_CHARACTER);
        waitForReadyRead();
        terminate();
        return true;
    }
    qWarning() << "Wrong server key: " << serverKey << " instead of " << savedKey;
    write("n");
    write(Platform::EOL_CHARACTER);
    terminate();
    return false;
}

void Plink::uploadToMetaFolder(const QFileInfo &file, bool append)
{
    qDebug() << "Plink::uploadToMetaFolder( " << file.absoluteFilePath() << ", " << append << " )";
    Settings *settings = Settings::getInstance();

    QString dest = settings->getServerUserName() + "@" + settings->getServerName();

    QStringList arguments;
    arguments << "-noagent";
    arguments << "-i";
    arguments << settings->createPrivatePuttyKeyFile();
    arguments << dest;
    arguments << "sh"
              << "-c";
    arguments << "cat";

    if (append) {
        arguments << ">>";
    } else {
        arguments << ">";
    }
    arguments << settings->getBackupRootFolder() + settings->getBackupPrefix() + "/"
                     + settings->getMetaFolderName() + "/" + file.fileName();

    createProcess(this->plinkName, arguments);
    setStandardInputFile(file.absoluteFilePath());
    start();
    waitForFinished();
}

bool Plink::generateKeys(const QString &password)
{
    qDebug() << "Plink::generateKeys( pw )";
    Settings *settings = Settings::getInstance();
    settings->deletePrivateKeyFiles();
    QString publicKeyFileName = settings->getAuthorizedKeyFolderName() + "key.public-openssh";
    QString privatePuttyKeyFileName = settings->getAuthorizedKeyFolderName() + "key.private-putty";
    QString privateOpenSshKeyFileName = settings->getAuthorizedKeyFolderName()
                                        + "key.private-openssh";
    QString authorizedKeyFileName = settings->getAuthorizedKeyFolderName()
                                    + settings->getAuthorizedKeyFileName();

    QStringList shellArguments(
        {QString("echo %1").arg(LOGIN_ECHO_MESSAGE),
         QString("puttygen -q -t %2 -b %3 -o %1 2>&1")
             .arg(privatePuttyKeyFileName, KEY_TYPE, KEY_LENGTH),
         QString("puttygen %1 -o %2 -O public-openssh 2>&1")
             .arg(privatePuttyKeyFileName, publicKeyFileName),
         QString("puttygen %1 -o %2 -O private-openssh 2>&1")
             .arg(privatePuttyKeyFileName, privateOpenSshKeyFileName),
         QString("cat %1 >> %2").arg(publicKeyFileName, authorizedKeyFileName),
         QString("echo %1").arg(START_PRIVATE_KEY_ECHO_MESSAGE),
         QString("cat %1").arg(privatePuttyKeyFileName),
         QString("echo %1").arg(END_PRIVATE_KEY_ECHO_MESSAGE),
         QString("echo %1").arg(START_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE),
         QString("cat %1").arg(privateOpenSshKeyFileName),
         QString("echo %1").arg(END_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE),
         QString("rm %1").arg(privatePuttyKeyFileName),
         QString("rm %1").arg(privateOpenSshKeyFileName),
         QString("rm %1").arg(publicKeyFileName)});

    QStringList arguments(
        {"-noagent",
         "-no-antispoof",
         QString("%1@%2").arg(settings->getServerUserName(), settings->getServerName()),
#ifdef Q_OS_WIN32
         "sh",
         "-c",
         StringUtils::quoteText(shellArguments.join(";"), "'")
#else
         StringUtils::quoteText("sh -c " + StringUtils::quoteText(shellArguments.join(";"), "'"),
                                "\"")
#endif
        });

// Workaround for Linux since password prompt does not appear on stdout
#ifndef Q_OS_WIN32
    QStringList plinkCommand;
    plinkCommand << this->plinkName;
    plinkCommand << arguments;
    QStringList scriptArguments(
        {"--quiet", "--return", "--log-out", "/dev/null", "--command", plinkCommand.join(" ")});
#endif

    emit infoSignal(QObject::tr("Log in ..."));
#ifdef Q_OS_WIN32
    createProcess(this->plinkName, arguments);
#else
    createProcess(this->scriptName, scriptArguments);
#endif
    start();

    while (true) {
        // the server terminates the connection immediately in case of an invalid username (but not
        // only then)
        if (!waitForReadyRead())
            throw LoginException(
                qApp->translate("Plink", "Error occurred during login, perhaps invalid username"));

        QString line = readAll();

        if (line.contains("password:", Qt::CaseInsensitive)) {
            qDebug() << "Password line: " << line;
            break;
        }

        qDebug() << "ignoring message from plink:" << line;
    }

    write(password.toLocal8Bit());
    write(Platform::EOL_CHARACTER);

    if (!waitForReadyRead())
        throw LoginException(qApp->translate("Plink", "Timeout occurred during login"));

    if (readAllStandardError().contains("Access denied")) {
        qWarning() << "Access denied. Username or password not valid";
        throw LoginException(qApp->translate("Plink", "Username or password not valid"));
    }

    QString line = readAll();
    line.replace("\n", "");
    line.replace("\r", "");
    qDebug() << "loggedInMessage1: " << line;
    if (line == "") {
        if (!waitForReadyRead())
            throw LoginException(qApp->translate("Plink", "Timeout occurred during login"));

        line = readAll();
        line.replace("\n", "");
        line.replace("\r", "");
        qDebug() << "loggedInMessage2: " << line;
    }

    if (line != LOGIN_ECHO_MESSAGE) {
        qWarning() << "Username or password not valid";
        throw LoginException(qApp->translate("Plink", "Username or password not valid"));
    }
    emit infoSignal(QObject::tr("Login successful"));
    emit infoSignal(QObject::tr("Generating key pair ... (This may take a while)"));
    qDebug() << "Login successful. Creating key pair ...";
    waitForReadyRead(10 * 60 * 1000); // 10 minutes

    write("\n"); // no passphrase

    waitForReadyRead();
    readAll();
    write("\n"); // repeat no passphrase

    // read the private keys (first key is putty, second is openssh)
    QStringList keyLines;
    while (blockingReadLine(&line)) {
        keyLines << line;
    }
    waitForFinished();

    QString privatePuttyKeyString = extractKey(keyLines,
                                               START_PRIVATE_KEY_ECHO_MESSAGE,
                                               END_PRIVATE_KEY_ECHO_MESSAGE);
    QString privateOpenSshKeyString = extractKey(keyLines,
                                                 START_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE,
                                                 END_PRIVATE_OPENSSH_KEY_ECHO_MESSAGE);

    if (!privatePuttyKeyString.startsWith(PUTTY_HEADER_FIRST_LINE)
        || !privateOpenSshKeyString.startsWith(OPEN_SSH_HEADER_FIRST_LINE)) {
        emit errorSignal(QObject::tr("Key has not been generated"));
        return false;
    }

    // save the keys
    settings->savePrivatePuttyKey(privatePuttyKeyString);
    settings->savePrivateOpenSshKey(privateOpenSshKeyString);

    emit infoSignal(QObject::tr("Key has been generated"));
    return true;
}

QString Plink::extractKey(const QStringList &keyLines,
                          const QString &startLine,
                          const QString &endLine)
{
    QString key;
    bool isKeyLine = false;
    foreach (QString line, keyLines) {
        // find start line
        if (!isKeyLine && line.startsWith(startLine)) {
            isKeyLine = true;
            continue;
        }

        // find end line
        if (isKeyLine && line.startsWith(endLine)) {
            break;
        }

        if (isKeyLine) {
            key.append(line);
        }
    }
    return key;
}

QList<int> Plink::getServerQuotaValues()
{
    qDebug() << "Plink::getServerQuotaValues()";
    Settings *settings = Settings::getInstance();

    QStringList arguments;
    arguments << "-noagent";
    arguments << "-i";
    arguments << settings->createPrivatePuttyKeyFile();
    arguments << settings->getServerUserName() + "@" + settings->getServerName();
    arguments << settings->getServerQuotaScriptName();

    createProcess(this->plinkName, arguments);
    start();
    waitForFinished();

    QByteArray quotaText = readAllStandardOutput();
    int quota, backup, snapshot;
    QTextStream in(quotaText);
    in >> quota >> backup >> snapshot;
    QList<int> sizes;
    sizes << quota << backup << snapshot;
    qDebug() << "Quota values on server:" << sizes;
    return sizes;
}

void Plink::testGenerateKeys()
{
    QString username = "ssbackup";
    QString password = "secret";
    Settings *settings = Settings::getInstance();
    settings->saveServerUserName(username);
    settings->saveServerName("localhost");
    Plink plink;
    qDebug() << plink.generateKeys(password);
}

void Plink::testLoginWithKey()
{
    Plink plink;
    qDebug() << plink.loginWithKey();
}

void Plink::testAssertCorrectFingerprint()
{
    Plink plink;
    plink.assertCorrectFingerprint();
}

namespace {
int dummy1 = TestManager::registerTest("testGenerateKeys", Plink::testGenerateKeys);
int dummy4 = TestManager::registerTest("testLoginWithKey", Plink::testLoginWithKey);
int dummy5 = TestManager::registerTest("testAssertCorrectFingerprint",
                                       Plink::testAssertCorrectFingerprint);
} // namespace
