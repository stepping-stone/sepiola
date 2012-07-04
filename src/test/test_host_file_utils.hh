/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2012 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#ifndef TEST_HOST_FILE_UTILS_HH
#define TEST_HOST_FILE_UTILS_HH

#include <QtTest/QtTest>

class TestHostFileUtils : public QObject
{
    Q_OBJECT
private slots:
    void getPuttyKey();
    void convertPuttyKey();
    void addPuttyKeyToOpenSSHKeyFile();
};

#endif // TEST_HOST_FILE_UTILS_HH
