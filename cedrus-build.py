#!/usr/bin/env python

import os, subprocess, time, sys, glob, shutil, datetime, platform


qt_binary_location = os.getenv( 'QT_BINARIES_REPO', '' )

if qt_binary_location == '':
    print 'FAIL. must set the evn var $QT_BINARIES_REPO to your checkout of https://github.com/cedrus/qt-binaries'
    print 'PLEASE NOTE: checkout qt-binaries, then run "set_env.py" (part of qt-binaries). That will set QT_BINARIES_REPO.'
    exit()

if ( False == os.path.exists( qt_binary_location ) ) or ( False == os.path.isdir( qt_binary_location ) ):
    print 'FAIL. script cannot verify that "' + str( qt_binary_location ) + '" ($QT_BINARIES_REPO) is a directory'
    exit()


def wait_for_process( p ):
    running_stuff = True
    while running_stuff:
        retcode = p.poll()
        if retcode is not None:
            running_stuff = False
            time.sleep(.05) # fraction of a second


build_command_01 = './configure -prefix ' + qt_binary_location + '/macosx/ -platform macx-g++-32 -no-framework'

p = subprocess.Popen(build_command_01, shell=True)

wait_for_process(p)

build_command_02 = 'make -j6 module-qtbase'

p = subprocess.Popen(build_command_02, shell=True)

wait_for_process(p)

build_command_03 = 'make -j6 install module-qtbase'

p = subprocess.Popen(build_command_03, shell=True)

wait_for_process(p)
