CONFIG -= x11 qt
SOURCES = hardfloat.cpp
OBJECTS_DIR = obj
QMAKE_CXXFLAGS += -MD

equals(QT_ARCH, "arm"):unix {
    test.commands = $${CROSS_COMPILE}readelf -A hardfloat | grep -q \'Tag_ABI_VFP_args: VFP registers\'
    test.depends = hardfloat
    QMAKE_EXTRA_TARGETS += test

    default.target = all
    default.depends += test
    QMAKE_EXTRA_TARGETS += default
}

