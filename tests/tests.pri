#****************************************************************************
#* Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
#* All rights reserved.
#* See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
#****************************************************************************

QT += testlib

HEADERS += \
    $$PWD/test_app.h \
    $$PWD/test_base.h \
    $$PWD/test_measure.h \

SOURCES += \
    $$PWD/runtests.cpp \
    $$PWD/test_app.cpp \
    $$PWD/test_base.cpp \
    $$PWD/test_measure.cpp \

# Copy input files
CONFIG += file_copies
COPIES += MayoTestsInputs
MayoTestsInputs.files = $$files($$PWD/inputs/*.*)
MayoTestsInputs.path = $$OUT_PWD/tests/inputs

mkpath($$OUT_PWD/tests/outputs)
