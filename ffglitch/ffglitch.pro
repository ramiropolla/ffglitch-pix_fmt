QT += widgets

HEADERS =               \
          ui_main.h     \
          ui_img.h      \
          ffglitch.h    \
          ffimage.h     \

SOURCES = main.cpp      \
          ui_main.cpp   \
          ui_img.cpp    \
          ffglitch.cpp  \
          ffimage.cpp   \

INCLUDEPATH += /opt/ffglitch/include
LIBS += -L/opt/ffglitch/lib -lavformat -lavcodec -lswscale -lavutil -lm
