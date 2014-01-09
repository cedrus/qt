/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtV8 module of the Qt Toolkit.
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

#ifndef CALCULATEHASH_P_H
#define CALCULATEHASH_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

// This is a reimplementation of V8's string hash algorithm.  It is significantly
// faster to do it here than call into V8, but it adds the maintenance burden of
// ensuring that the two hashes are identical.  We Q_ASSERT() that the two return
// the same value.  If these asserts start to fail, the hash code needs to be
// synced with V8.
namespace HashedString {
    static const int kMaxArrayIndexSize = 10;
    static const int kMaxHashCalcLength = 16383;
    static const int kNofHashBitFields = 2;
    static const int kHashShift = kNofHashBitFields;
    static const int kIsNotArrayIndexMask = 1 << 1;
    static const int kArrayIndexValueBits = 24;
    static const int kArrayIndexHashLengthShift = kArrayIndexValueBits + kNofHashBitFields;
    static const int kMaxCachedArrayIndexLength = 7;
};

template <typename schar>
uint32_t calculateHash(const schar* chars, int length) {
    if (length > HashedString::kMaxHashCalcLength) {
        // V8 trivial hash
        return (length << HashedString::kHashShift) | HashedString::kIsNotArrayIndexMask;
    }

    uint32_t raw_running_hash = 0;
    uint32_t array_index = 0;
    bool is_array_index = (0 < length && length <= HashedString::kMaxArrayIndexSize);
    bool is_first_char = true;

    int ii = 0;
    for (;is_array_index && ii < length; ++ii) {
        quint32 c = *chars++;

        raw_running_hash += c;
        raw_running_hash += (raw_running_hash << 10);
        raw_running_hash ^= (raw_running_hash >> 6);

        if (c < '0' || c > '9') {
            is_array_index = false;
        } else {
            int d = c - '0';
            if (is_first_char) {
                is_first_char = false;
                if (c == '0' && length > 1) {
                    is_array_index = false;
                    continue;
                }
            }
            if (array_index > 429496729U - ((d + 2) >> 3)) {
                is_array_index = false;
            } else {
                array_index = array_index * 10 + d;
            }
        }
    }

    for (;ii < length; ++ii) {
        raw_running_hash += *chars++;
        raw_running_hash += (raw_running_hash << 10);
        raw_running_hash ^= (raw_running_hash >> 6);
    }

    if (is_array_index) {
        array_index <<= HashedString::kHashShift;
        array_index |= length << HashedString::kArrayIndexHashLengthShift;
        return array_index;
    } else {
        raw_running_hash += (raw_running_hash << 3);
        raw_running_hash ^= (raw_running_hash >> 11);
        raw_running_hash += (raw_running_hash << 15);
        if (raw_running_hash == 0) {
            raw_running_hash = 27;
        }

        return (raw_running_hash << HashedString::kHashShift) | HashedString::kIsNotArrayIndexMask;
    }
}

QT_END_NAMESPACE

#endif // CALCULATEHASH_P_H
