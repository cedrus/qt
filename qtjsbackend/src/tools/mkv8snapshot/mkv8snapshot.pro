option(host_build)
TARGET          = mkv8snapshot$$qtPlatformTargetSuffix()
CONFIG          -= qt app_bundle
CONFIG          += console warn_off

DESTDIR         = ../../../bin
LIBS            =
OBJECTS_DIR     = .

contains(QT_CONFIG, build_all): CONFIG += build_all
win32|mac:!macx-xcode: CONFIG += debug_and_release

include(../../v8/v8.pri)

cross_compile {
    equals(V8_TARGET_ARCH, arm): SOURCES += $$V8SRC/arm/simulator-arm.cc
}

SOURCES += \
    $$V8SRC/snapshot-empty.cc \
    $$V8SRC/mksnapshot.cc

unix:LIBS += -lpthread

# We don't need to install this tool, it's only used for building v8.
# However we do have to make sure that 'make install' builds it.
dummytarget.CONFIG = dummy_install
INSTALLS += dummytarget
