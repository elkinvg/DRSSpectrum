TEMPLATE = app
CONFIG += console
CONFIG -= qt

MOC_DIR = moc
OBJECTS_DIR = obj
DESTDIR += bin
DEPENDPATH += dll
CONFIG += debug_and_release

win32 {
INCLUDEPATH += c:\root\include\
}

unix {
}

SOURCES += main.cpp \
    drsread.cpp \
    drs4outfile.cpp \
    drstype.cpp \
    drssignalproc.cpp \
    drsreadoneline.cpp \
    drsreadoffline.cpp

OBJECTIVE_SOURCES += ./hlp/auto_signal_detect.c \
#SOURCES += ./hlp/auto_signal_detect.c \
           ./hlp/calibrate_pixels.cpp \
           ./hlp/domhistgcc.cpp \
           ./hlp/domino-n.cpp \
           ./hlp/drs_reader.cpp \
           ./hlp/fit_spectra.cpp \
           ./hlp/fit_spectra_complex.cpp \
           ./hlp/generate_spectra.cpp \
           ./hlp/get_spectrum.cxx \
           ./hlp/hist.C \
           ./hlp/process_cluster_log.cpp \
           ./hlp/read_data_vah.cpp \
           ./hlp/qnetconv.cc \
           ./hlp/main_get_opt.c


HEADERS += \ 
    drsread.h \
    drs4outfile.h \
    drstype.h \
    drssignalproc.h \
    drsreadoneline.h \
    drsreadoffline.h
