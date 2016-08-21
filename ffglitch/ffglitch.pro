QT += widgets

HEADERS =                   \
          ui_main.h         \
          ui_img.h          \
          ffglitch.h        \
          ffimage.h         \
          py_ffglitch.h     \

SOURCES = main.cpp          \
          ui_main.cpp       \
          ui_img.cpp        \
          ffglitch.cpp      \
          ffimage.cpp       \
          py_ffglitch.cpp   \

INCLUDEPATH += /opt/ffglitch/include
QMAKE_CXXFLAGS += `python2.7-config --cflags | sed s/"-Wstrict-prototypes"//` -Wno-write-strings
LIBS += -L/opt/ffglitch/lib -lavformat -lavcodec -lswscale -lavutil -lm `python2.7-config --ldflags`
