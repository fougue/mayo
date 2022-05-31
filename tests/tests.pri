QT += testlib

HEADERS += \
    $$PWD/test_app.h \
    $$PWD/test_base.h \

SOURCES += \
    $$PWD/runtests.cpp \
    $$PWD/test_app.cpp \
    $$PWD/test_base.cpp \

# Copy input files
CONFIG += file_copies
COPIES += MayoTestsInputs
MayoTestsInputs.files = $$files($$PWD/inputs/*.*)
MayoTestsInputs.path = $$OUT_PWD/tests/inputs
