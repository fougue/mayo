# Return the input path re-written using the system-dependent separator
defineReplace(sysPath) {
  win*:result = $$replace(1, /, \\)
  else:result = $$1
  return($$result)
}

INCLUDEPATH += $$CASCADE_INC_DIR

linux-*:DEFINES += \
    HAVE_CONFIG_H \
    HAVE_FSTREAM \
    HAVE_IOSTREAM \
    HAVE_IOMANIP \
    HAVE_LIMITS_H

win32-*:DEFINES += WNT
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

message(OCC_VERSION : $$OCC_VERSION_STR)

# Platform dependant config
equals(QT_ARCH, i386) {
  ARCH_BITS_SIZE = 32
} else:equals(QT_ARCH, x86_64) {
  ARCH_BITS_SIZE = 64
  DEFINES += _OCC64
}
else {
  error(Platform architecture not supported (QT_ARCH = $$QT_ARCH))
}

LIBS += $$sysPath($$join(CASCADE_LIB_DIR, " -L", -L))
QMAKE_RPATHDIR += $$CASCADE_LIB_DIR
