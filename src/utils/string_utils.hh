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

#ifndef STRING_UTILS_HH_
#define STRING_UTILS_HH_

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>

#include "test/test_manager.hh"
#include "utils/datatypes.hh"

using namespace std;

class StringUtils
{
public:
    static QString encaps(const QString &aStr,
                          const QString &beforeTok = "",
                          const QString &afterTok = "");
    static QString concatSep(const QString &aStr,
                             const QString &bStr = "",
                             const QString &tok = ",");
    //	static QString filenameShrink(const QString& filename, const int maxlen);
    static QString filenameShrink(const QString &filename,
                                  const int maxlen,
                                  const QChar dirSeparator = QDir::separator());
    static QString quoteText(QString text,
                             QString tok,
                             QString escapeCharacter = "\\",
                             bool bashSaveSingleQuoteHandling = true);
    static QString buf2QString(const char *buf);
    static QString buf2QString(QString buf);

    static float trafficStr2BytesPerSecond(QString trafficStr);
    static QString bytesToReadableStr(double traffic, const QString &unitTxt);
    static QString bytesToReadableStr(quint64 traffic, const QString &unitTxt);
    static QString kBytesToReadableStr(double traffic, const QString &unitTxt);

    static QString equalStart(const QString &aStr, const QString &bStr);

    static bool isFilename(
        const QString &txt); // simple guess if txt can be displayed compact like a filename
    static bool isDirectoryName(const QString &txt); // simple guess if txt might be a folder name
    static QString dirPart(const QString &filename); // for folders ending with "/" returns self
    static QString parentDir(
        const QString &filename); // same as dirPart, except for folders -> one back and not self
    static QString equalDirPart(const QString &filenameA, const QString &filenameB);
    static bool writeStringListToFile(const QList<QString> &include_dirs_list,
                                      const QString &include_dirs_filename,
                                      QString eolChar = "\n");
    static bool writeQByteArrayListToFile(const QList<QByteArray> &include_dirs_list,
                                          const QString &include_dirs_filename,
                                          QByteArray eolChar = "\n");

    static void testChar2StdString();
    static void testQuoteText();

    static QString fromLocalEnc(QByteArray byteArray);

private:
    static QString unit_prefixes() { return QString(" kMGTPEZY"); };
};

namespace {
// run test by calling sepiola with parameter: -test testChar2StdString
int stringUtils_dummy1 = TestManager::registerTest("testChar2StdString",
                                                   StringUtils::testChar2StdString);
int stringUtils_dummy2 = TestManager::registerTest("testQuoteText", StringUtils::testQuoteText);
} // namespace

inline void StringUtils::testChar2StdString()
{
    QString testStr = QString("abcd");
    char *myText = testStr.toLatin1().data();
    std::string myString1(myText);
    cout << myString1;
    std::string myString2 = myText;
    cout << myString2;
    cout << std::string(myText) << endl;
}

inline void StringUtils::testQuoteText()
{
    QStringList testTasks;
    QStringList testSolutions;
    QStringList testResults;

    testTasks.append("It's a text with single quotes in it.");
    testSolutions.append("'It'\\''s a text with single quotes in it.'");
    testResults.append(quoteText(testTasks.last(), "'"));

    testTasks.append("...and one with a single quote at the end'");
    testSolutions.append("'...and one with a single quote at the end'\\'");
    testResults.append(quoteText(testTasks.last(), "'"));

    testTasks.append("'''''...and the last with \"double\" quotes and 'single' ones.'''''");
    testSolutions.append("\\'\\'\\'\\'\\''...and the last with \"double\" quotes and "
                         "'\\''single'\\'' ones.'\\'\\'\\'\\'\\'");
    testResults.append(quoteText(testTasks.last(), "'"));

    for (int i = 0; i < testTasks.size(); i++) {
        bool success = testSolutions.at(i) == testResults.at(i);
        qDebug() << (success ? "test successful:" : "test failed:   ")
                 << "quoteText(\"" + testTasks.at(i) + "\")" << QString(" = \n  ")
                 << testResults.at(i) << QString(" ") + (success ? "==" : "!=") + " \n  "
                 << testSolutions.at(i);
    }
}

inline QString StringUtils::encaps(const QString &aStr,
                                   const QString &beforeTok,
                                   const QString &afterTok)
{
    if (aStr == "")
        return aStr;
    return beforeTok + aStr + afterTok;
}

inline QString StringUtils::concatSep(const QString &aStr, const QString &bStr, const QString &tok)
{
    if (aStr == "")
        return bStr;
    if (bStr == "")
        return aStr;
    return aStr + tok + bStr;
}

inline QString StringUtils::filenameShrink(const QString &filename,
                                           const int maxlen,
                                           const QChar dirSeparator)
{
    int len = filename.length();
    if (len <= maxlen) {
        return (filename);
    } else {
        QString ellipses = QString("...");
        int fn_start = filename.lastIndexOf(dirSeparator, -2);
        QString fn_only = filename.mid(fn_start + 1);
        if (len - fn_start + ellipses.length() > maxlen) // filename only already too long
        {
            return ((fn_only.length() > maxlen)
                        ? (fn_only.left(maxlen - ellipses.length()) + ellipses)
                        : fn_only);
        } else {
            return (filename.left(maxlen - len + fn_start - ellipses.length()) + ellipses
                    + dirSeparator + fn_only);
        }
    }
}

inline QString StringUtils::quoteText(QString text,
                                      QString quote,
                                      QString escapeCharacter,
                                      bool bashSaveSingleQuoteHandling)
{
    quote = quote.at(0);
    escapeCharacter = escapeCharacter.at(0);
    if (quote != escapeCharacter) {
        if (bashSaveSingleQuoteHandling && quote == "'") {
            QStringList textParts = text.split(quote, QString::KeepEmptyParts);
            QStringList quotedParts;
            for (int i = 0; i < textParts.size(); i++) {
                QString textPart = textParts.at(i);
                textParts[i] = encaps(textPart
                                          .replace(escapeCharacter,
                                                   escapeCharacter + escapeCharacter)
                                          .replace(quote, escapeCharacter + quote),
                                      quote,
                                      quote);
            }
            return (textParts.join(escapeCharacter + quote));
        } else {
            return (encaps(text.replace(escapeCharacter, escapeCharacter + escapeCharacter)
                               .replace(quote, escapeCharacter + quote),
                           quote,
                           quote));
        }
    } else {
        return text;
    }
}

inline QString StringUtils::buf2QString(const char *buf)
{
    int i = 0;
    QString retStr;
    while (buf[i] != (char) 0) {
        if ((uchar) buf[i] >= 32 && (uchar) buf[i] <= 128) {
            retStr.append(QChar(buf[i]));
        } else {
            QString num;
            num.setNum((int) (uchar) buf[i], 16);
            retStr.append(QString("#%1;").arg(num));
        }
        i++;
    }
    return (retStr);
}

inline QString StringUtils::buf2QString(QString buf)
{
    return buf2QString(buf.toLatin1().data());
}

inline float StringUtils::trafficStr2BytesPerSecond(QString trafficStr)
{
    int numLen = trafficStr.indexOf("B/s"), pos;
    if (numLen != -1) {
        QChar expChar = trafficStr.toUpper().at(numLen - 1);
        if (QString("0123456789").contains(expChar)) { // only B/s _not_ KB/s, MB/s etc.
            return trafficStr.left(numLen).toFloat();
        } else {
            if ((pos = StringUtils::unit_prefixes().toUpper().indexOf(expChar)) != -1) {
                return pow(1024, pos) * trafficStr.left(numLen - 1).toFloat();
            } else {
                return 0.0f;
            }
        }
    } else {
        return 0.0f;
    }
}

inline QString StringUtils::bytesToReadableStr(double traffic, const QString &unitTxt)
{
    QString prefixes = StringUtils::unit_prefixes();
    int i = 0;
    while (traffic > 1000.0) {
        traffic /= 1024.0;
        i++;
    }
    QString retVal, prefix = prefixes.mid(i, 1).trimmed();
    return retVal.setNum(traffic, 'f', (prefix.length() > 0) ? 2 : 0) + " " + prefix + unitTxt;
}

inline QString StringUtils::bytesToReadableStr(quint64 traffic, const QString &unitTxt)
{
    return bytesToReadableStr((double) traffic, unitTxt);
}

inline QString StringUtils::kBytesToReadableStr(double traffic, const QString &unitTxt)
{
    return bytesToReadableStr(((double) traffic) * 1024.0, unitTxt);
}

inline QString StringUtils::equalStart(const QString &aStr, const QString &bStr)
{
    int i = 0, imax = std::min<int>(aStr.length(), bStr.length());
    while (i < imax && aStr[i] == bStr[i])
        i++;
    return aStr.left(i);
}

inline bool StringUtils::isFilename(const QString &txt)
{
    return (txt.count("/") >= 2);
}

inline bool StringUtils::isDirectoryName(const QString &txt)
{
    return isFilename(txt) && txt.endsWith("/");
}

inline QString StringUtils::dirPart(const QString &filename)
{ // for folders ending with "/" returns self
    return filename.left(filename.lastIndexOf("/") + 1);
}
inline QString StringUtils::parentDir(const QString &filename)
{ // same as dirPart, except for folders -> one back and not self
    return filename.left(filename.lastIndexOf("/", filename.length() - 2) + 1);
}

inline QString StringUtils::equalDirPart(const QString &filenameA, const QString &filenameB)
{
    return dirPart(equalStart(filenameA, filenameB));
}

inline bool StringUtils::writeStringListToFile(const QList<QString> &item_list,
                                               const QString &filename,
                                               QString eolChar)
{
    QFile out(filename);
    if (out.open(QIODevice::WriteOnly)) {
        QTextStream outStream(&out);
        foreach (QString item, item_list) {
            outStream << item + eolChar;
        }
        out.close();
        return true;
    } else {
        qDebug() << "StringUtils::writeStringListToFile(...)"
                 << "problems opening file:" << filename;
        return false;
    }
}

inline bool StringUtils::writeQByteArrayListToFile(const QList<QByteArray> &item_list,
                                                   const QString &filename,
                                                   QByteArray eolChar)
{
    QFile out(filename);
    if (out.open(QIODevice::WriteOnly)) {
        foreach (QByteArray item, item_list) {
            out.write(item + eolChar);
        }
        out.close();
        return true;
    } else {
        qDebug() << "StringUtils::writeStringListToFile(...)"
                 << "problems opening file:" << filename;
        return false;
    }
}

#endif /*STRING_UTILS_HH_*/
