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

# where the main qt dylibs are:
# /Users/kkheller/Documents/qt5_bins/macosx/lib

# other ones found (RECURSIVELY) here:
# /Users/kkheller/Documents/qt5_bins/macosx/plugins

what_to_walk = qt_binary_location + os.sep + 'macosx' + os.sep + 'plugins'
where_to_move_to = qt_binary_location + os.sep + 'macosx' + os.sep + 'lib' + os.sep

for root, sub_folders, files in os.walk( what_to_walk ):

    for f in files:
        source = os.path.join(root, f)
        destination = os.path.join( where_to_move_to, f )
        os.rename(source, destination)

# remove SYMLINKS from the final 'lib' folder:
for root, sub_folders, files in os.walk( where_to_move_to ):

    for f in files:
        if os.path.islink( os.path.join(root, f) ):
            os.remove( os.path.join(root, f) )


# now run install_name_tool to update all mac dylibs to @executable_path
command_04 = str( qt_binary_location + os.sep
                  + 'macosx' + os.sep
                  + 'cedrus_qt_mach-o_dylib_fixer.sh '
                  + where_to_move_to + ' be_verbose' )

p = subprocess.Popen(command_04, shell=True)

wait_for_process(p)

