TEMPLATE = subdirs
SUBDIRS =
contains(QT_CONFIG, v8snapshot): SUBDIRS += mkv8snapshot
