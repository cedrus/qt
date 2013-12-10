TEMPLATE = lib
CONFIG += plugin
SOURCES = plugin.cpp
QT = core declarative
DESTDIR = ../imports/com/nokia/AutoTestQmlPluginType

include(../qmldir_copier.pri)

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
