TEMPLATE      = subdirs
CONFIG += no_docs_target

SUBDIRS = \
    gestures \
    gui \
    opengl \
    qpa \
    qtestlib \
    threads \
    tools \
    touch \
    widgets

aggregate.files = aggregate/examples.pro
aggregate.path = $$[QT_INSTALL_EXAMPLES]
readme.files = README
readme.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += aggregate readme
