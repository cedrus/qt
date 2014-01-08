TEMPLATE = subdirs

qtHaveModule(widgets): SUBDIRS += svggenerator  svgviewer
SUBDIRS += richtext draganddrop 

qtHaveModule(opengl):!contains(QT_CONFIG, opengles2): SUBDIRS += opengl
