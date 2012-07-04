/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#ifndef TEST_POSIX_ACL_H
#define TEST_POSIX_ACL_H

#include <QtTest/QtTest>

class TestPosixAcl: public QObject
{
    Q_OBJECT
private slots:
    void unescapeOctalCharacters();
    void getMetadata();
    void setMetadata();
};

#endif
