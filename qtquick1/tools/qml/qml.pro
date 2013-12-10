CONFIG += uic declarative_debug declarative

include(qml.pri)

SOURCES += main.cpp

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

wince* {
    QT += xml

    qtHaveModule(scripttools) {
        QT += scripttools
    }
    contains(QT_CONFIG, phonon) {
        QT += phonon
    }
    qtHaveModule(xmlpatterns) {
        QT += xmlpatterns
    }
    qtHaveModule(webkitwidgets) {
        QT += webkitwidgets
    }
}

mac {
    QMAKE_INFO_PLIST=Info_mac.plist
    TARGET=QMLViewer
    ICON=qml.icns
} else {
    TARGET=qmlviewer
}

load(qt_app)
