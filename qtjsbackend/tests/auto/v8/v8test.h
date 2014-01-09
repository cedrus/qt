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

#ifndef V8TEST_H
#define V8TEST_H

#ifdef QT_CORE_LIB
#include <private/v8.h>
#else
#include <v8.h>
#endif

bool v8test_eval();
bool v8test_evalwithinwith();
bool v8test_userobjectcompare();
bool v8test_externalteardown();
bool v8test_globalcall();
bool v8test_getcallingqmlglobal();
bool v8test_typeof();
bool v8test_referenceerror();
bool v8test_qtbug_24871();
bool v8test_fallbackpropertyhandler_callbacks();
bool v8test_fallbackpropertyhandler_in_prototype();
bool v8test_fallbackpropertyhandler_nonempty();
bool v8test_completehash();
bool v8test_stringhashcomparison();
bool v8test_qmlmodevariables();
bool v8test_qmlmodeinlinelocal();

#endif // V8TEST_H

