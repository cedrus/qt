#!/bin/bash
#############################################################################
##
## Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
## Contact: http://www.qt-project.org/legal
##
## This file is the build configuration utility of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and Digia.  For licensing terms and
## conditions see http://qt.digia.com/licensing.  For further information
## use the contact form at http://qt.digia.com/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 2.1 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU Lesser General Public License version 2.1 requirements
## will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## In addition, as a special exception, Digia gives you certain additional
## rights.  These rights are described in the Digia Qt LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3.0 as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU General Public License version 3.0 requirements will be
## met: http://www.gnu.org/copyleft/gpl.html.
##
##
## $QT_END_LICENSE$
##
#############################################################################

die() {
    echo $*
    exit 1
}

if [ $# -eq 2 ]; then
    repository=$1
    tag=$2
elif [ $# -eq 3 ]; then
    repository=$1
    tag=$2
    rev=$3
else
    die "usage: $0 [url] [commit] ([hash])"
fi

require_clean_work_tree() {
    # test if working tree is dirty
    git rev-parse --verify HEAD > /dev/null &&
    git update-index --refresh &&
    git diff-files --quiet &&
    git diff-index --cached --quiet HEAD ||
    die "Working tree is dirty"
}

test -z "$(git rev-parse --show-cdup)" || {
       exit=$?
       echo >&2 "You need to run this command from the toplevel of the working tree."
       exit $exit
}

echo "checking working tree"
require_clean_work_tree

echo "fetching"
git fetch $repository $tag
if [ $? != 0 ]; then
    die "git fetch failed"
fi

if [ -z $rev ]; then
    rev=`git rev-parse FETCH_HEAD`
fi

srcdir=src/3rdparty/v8
absSrcDir=$PWD/$srcdir
localDiff=

echo "replacing $srcdir"
if [  -d $srcdir ]; then
    git ls-files $srcdir | xargs rm
    git ls-files -z $srcdir | git update-index --force-remove -z --stdin
    lastImport=`git rev-list --max-count=1 HEAD -- $srcdir/ChangeLog`
    changes=`git rev-list --no-merges --reverse $lastImport.. -- $srcdir`
    localDiff=/tmp/v8_patch
    echo -n>$localDiff
    for change in $changes; do
        echo "Saving commit $change"
        git show -p --stat "--pretty=format:%nFrom %H Mon Sep 17 00:00:00 2001%nFrom: %an <%ae>%nDate: %ad%nSubject: %s%n%b%n" $change -- $srcdir >> $localDiff
        echo "-- " >> $localDiff
        echo "1.2.3" >> $localDiff
        echo >> $localDiff
    done
    if [ -s $localDiff ]; then
        echo "Saved locally applied patches to $localDiff"
    else
        localDiff=""
    fi
else
    mkdir -p $srcdir
fi

git read-tree --prefix=$srcdir $rev
if [ $? != 0 ]; then
    die "Invalid hash!"
fi
git checkout $srcdir

cat >commitlog.txt <<EOT
Updated V8 from $repository to $rev
EOT

echo "Changes:"
echo
git --no-pager diff --name-status --cached $srcdir

echo
echo "Wrote commitlog.txt. Use with"
echo
echo "    git commit -e -F commitlog.txt"
echo
echo "to commit your changes"

if [ -n "$localDiff" ]; then
    echo
    echo "The Qt specific modifications to V8 are now stored as a git patch in $localDiff"
    echo "You may want to appy them with"
    echo
    echo "    git am -3 $localDiff"
    echo
fi
