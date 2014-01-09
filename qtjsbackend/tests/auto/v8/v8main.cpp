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

#include "v8test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static int exit_status = EXIT_SUCCESS;

#define RUN_TEST(testname) { \
    if (argc == 1 || 0 == ::strcmp(argv[1], # testname)) { \
        if (!v8test_ ## testname()) { \
            printf ("Test %s FAILED\n", # testname); \
            exit_status = EXIT_FAILURE; \
        } else \
            printf ("Test %s PASS\n", # testname); \
    } \
}

int main(int argc, char *argv[])
{
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

    RUN_TEST(eval);
    RUN_TEST(evalwithinwith);
    RUN_TEST(userobjectcompare);
    RUN_TEST(externalteardown);
    RUN_TEST(globalcall);
    RUN_TEST(getcallingqmlglobal);
    RUN_TEST(typeof);
    RUN_TEST(referenceerror);
    RUN_TEST(qtbug_24871);
    RUN_TEST(fallbackpropertyhandler_callbacks);
    RUN_TEST(fallbackpropertyhandler_in_prototype);
    RUN_TEST(fallbackpropertyhandler_nonempty);
    RUN_TEST(completehash);
    RUN_TEST(qmlmodevariables);
    RUN_TEST(qmlmodeinlinelocal);

    return exit_status;
}
