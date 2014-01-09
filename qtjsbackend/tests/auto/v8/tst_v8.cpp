/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <qtest.h>
#include <private/v8.h>
#include "v8test.h"

using namespace v8;

class tst_v8 : public QObject
{
    Q_OBJECT
public:
    tst_v8() {}

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void eval();
    void evalwithinwith();
    void userobjectcompare();
    void externalteardown();
    void globalcall();
    void getcallingqmlglobal();
    void typeOf();
    void referenceerror();
    void qtbug_24871();
    void fallbackpropertyhandler_callbacks();
    void fallbackpropertyhandler_in_prototype();
    void fallbackpropertyhandler_nonempty();
    void completehash();
    void stringhashcomparison();
    void qmlmodevariables();
    void qmlmodeinlinelocal();
};

void tst_v8::eval()
{
    QVERIFY(v8test_eval());
}

void tst_v8::evalwithinwith()
{
    QVERIFY(v8test_evalwithinwith());
}

void tst_v8::userobjectcompare()
{
    QVERIFY(v8test_userobjectcompare());
}

void tst_v8::externalteardown()
{
    QVERIFY(v8test_externalteardown());
}

void tst_v8::globalcall()
{
    QVERIFY(v8test_globalcall());
}

void tst_v8::getcallingqmlglobal()
{
    QVERIFY(v8test_getcallingqmlglobal());
}

void tst_v8::typeOf()
{
    QVERIFY(v8test_typeof());
}

void tst_v8::referenceerror()
{
    QVERIFY(v8test_referenceerror());
}

void tst_v8::qtbug_24871()
{
    QVERIFY(v8test_qtbug_24871());
}

void tst_v8::fallbackpropertyhandler_callbacks()
{
    QVERIFY(v8test_fallbackpropertyhandler_callbacks());
}

void tst_v8::fallbackpropertyhandler_in_prototype()
{
    QVERIFY(v8test_fallbackpropertyhandler_in_prototype());
}

void tst_v8::fallbackpropertyhandler_nonempty()
{
    QVERIFY(v8test_fallbackpropertyhandler_nonempty());
}

void tst_v8::completehash()
{
    QVERIFY(v8test_completehash());
}

void tst_v8::stringhashcomparison()
{
    QVERIFY(v8test_stringhashcomparison());
}

void tst_v8::qmlmodevariables()
{
    QVERIFY(v8test_qmlmodevariables());
}

void tst_v8::qmlmodeinlinelocal()
{
    QVERIFY(v8test_qmlmodeinlinelocal());
}

int main(int argc, char *argv[])
{
    V8::SetFlagsFromCommandLine(&argc, argv, true);

    QCoreApplication app(argc, argv);
    tst_v8 tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_v8.moc"
