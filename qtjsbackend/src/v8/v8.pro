TARGET     = QtV8
QT         =
CONFIG += internal_module

!contains(QT_CONFIG, static): MODULE_DEFINES += V8_SHARED USING_V8_SHARED

win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000

# for -std=c++0x, MinGW time.h does not declare _tzset
win32-g++*:QMAKE_CXXFLAGS_CXX11 = -std=gnu++0x

load(qt_module)
CONFIG += warn_off

# Remove includepaths that were added by qt_module.
# These cause compilation of V8 to fail because they appear before
# 3rdparty/v8/src; 3rdparty/v8/src/v8.h will then be "shadowed" by
# the public v8.h API header (they are not the same!).
INCLUDEPATH -= $$MODULE_PRIVATE_INCLUDES
INCLUDEPATH -= $$MODULE_PRIVATE_INCLUDES/$$TARGET
INCLUDEPATH -= $$MODULE_INCLUDES $$MODULE_INCLUDES/..

!contains(QT_CONFIG, static): DEFINES += V8_SHARED BUILDING_V8_SHARED

include(v8.pri)

contains(QT_CONFIG, v8snapshot) {
    mkv8snapshot.commands = ../../bin/mkv8snapshot$$qtPlatformTargetSuffix() ${QMAKE_FILE_OUT}
    DUMMY_FILE = v8.pro
    mkv8snapshot.input = DUMMY_FILE
    mkv8snapshot.output = $$V8_GENERATED_SOURCES_DIR/snapshot.cpp
    mkv8snapshot.variable_out = SOURCES
    mkv8snapshot.dependency_type = TYPE_C
    mkv8snapshot.name = generating[v8] ${QMAKE_FILE_IN}
    silent:mkv8snapshot.commands = @echo generating[v8] ${QMAKE_FILE_IN} && $$mkv8snapshot.commands
    QMAKE_EXTRA_COMPILERS += mkv8snapshot
} else {
    SOURCES += $$V8SRC/snapshot-empty.cc
}
