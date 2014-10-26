TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../brush.c \
    ../bsp_wad.c \
    ../bsp2prt.c \
    ../bspfile.c \
    ../bspinfo.c \
    ../cmdlib.c \
    ../csg4.c \
    ../faces.c \
    ../level_main.c \
    ../light.c \
    ../light_face.c \
    ../light_trace.c \
    ../map.c \
    ../mathlib.c \
    ../mem.c \
    ../outside.c \
    ../portals.c \
    ../scriptlib.c \
    ../solidbsp.c \
    ../threads.c \
    ../tjunc.c \
    ../tree.c \
    ../vis.c \
    ../vis_flow.c \
    ../vis_sound.c \
    ../winding.c \
    ../writebsp.c

include(deployment.pri)
qtcAddDeployment()

OTHER_FILES +=

HEADERS += \
    ../bsp5.h \
    ../bspfile.h \
    ../cmdlib.h \
    ../light.h \
    ../map.h \
    ../mathlib.h \
    ../mem.h \
    ../resource.h \
    ../threads.h \
    ../vis.h \
    ../winding.h

