#****************************************************************************
#* Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
#* All rights reserved.
#* See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
#****************************************************************************

# Declare pure QMake variables out of OCC envvars
isEmpty(CASCADE_INC_DIR):CASCADE_INC_DIR = $$(CSF_OCCTIncludePath)
isEmpty(CASCADE_LIB_DIR):CASCADE_LIB_DIR = $$(CSF_OCCTLibPath)
isEmpty(CASCADE_BIN_DIR):CASCADE_BIN_DIR = $$(CSF_OCCTBinPath)
isEmpty(CASCADE_SRC_DIR):CASCADE_SRC_DIR = $$(CSF_OCCTResourcePath)
equals(QT_ARCH, i386) {
    isEmpty(CASCADE_OPTBIN_DIRS):CASCADE_OPTBIN_DIRS = $$(CSF_OPT_BIN32)
} else:equals(QT_ARCH, x86_64) {
    isEmpty(CASCADE_OPTBIN_DIRS):CASCADE_OPTBIN_DIRS = $$(CSF_OPT_BIN64)
}
isEmpty(CASCADE_DEFINES):CASCADE_DEFINES = $$(CSF_DEFINES)

INCLUDEPATH += $$CASCADE_INC_DIR

linux-*:DEFINES += \
    HAVE_CONFIG_H \
    HAVE_FSTREAM \
    HAVE_IOSTREAM \
    HAVE_IOMANIP \
    HAVE_LIMITS_H

DEFINES += $$split(CASCADE_DEFINES, ;)
DEFINES += OCCT_HANDLE_NOCAST
linux-*:DEFINES += LIN LININTEL OCC_CONVERT_SIGNALS

# Find OCC version
OCC_VERSION_FILE_CONTENTS = $$cat($$CASCADE_INC_DIR/Standard_Version.hxx, lines)

OCC_VERSION_MAJOR = $$find(OCC_VERSION_FILE_CONTENTS, OCC_VERSION_MAJOR\s+[0-9]+)
OCC_VERSION_MAJOR = $$section(OCC_VERSION_MAJOR, " ", -1)
OCC_VERSION_MINOR = $$find(OCC_VERSION_FILE_CONTENTS, OCC_VERSION_MINOR\s+[0-9]+)
OCC_VERSION_MINOR = $$section(OCC_VERSION_MINOR, " ", -1)
OCC_VERSION_PATCH = $$find(OCC_VERSION_FILE_CONTENTS, OCC_VERSION_MAINTENANCE\s+[0-9]+)
OCC_VERSION_PATCH = $$section(OCC_VERSION_PATCH, " ", -1)

OCC_VERSION_STR = $$join($$list($$OCC_VERSION_MAJOR, $$OCC_VERSION_MINOR, $$OCC_VERSION_PATCH), .)

# Platform dependent config
equals(QT_ARCH, i386) {
    # NOP
} else:equals(QT_ARCH, x86_64) {
    DEFINES += _OCC64
} else {
    error(Platform architecture not supported (QT_ARCH = $$QT_ARCH))
}

LIBS += $$system_path($$join(CASCADE_LIB_DIR, " -L", -L))
QMAKE_RPATHDIR += $$CASCADE_LIB_DIR

defineTest(minOpenCascadeVersion) {
    maj = $$1
    min = $$2
    patch = $$3
    isEqual(OCC_VERSION_MAJOR, $$maj) {
        isEqual(OCC_VERSION_MINOR, $$min) {
            isEqual(OCC_VERSION_PATCH, $$patch) {
                return(true)
            }
            greaterThan(OCC_VERSION_PATCH, $$patch) {
                return(true)
            }
        }
        greaterThan(OCC_VERSION_MINOR, $$min) {
            return(true)
        }
    }
    greaterThan(OCC_VERSION_MAJOR, $$maj) {
        return(true)
    }
    return(false)
}
