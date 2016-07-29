# Return the input path re-written using the system-dependent separator
defineReplace(sysPath) {
  win*:result = $$replace(1, /, \\)
  else:result = $$1
  return($$result)
}

INCLUDEPATH += $$CASCADE_ROOT/inc

linux-*:DEFINES += HAVE_CONFIG_H \
                   HAVE_FSTREAM \
                   HAVE_IOSTREAM \
                   HAVE_IOMANIP \
                   HAVE_LIMITS_H

win32-*:DEFINES += WNT
linux-*:DEFINES += LIN LININTEL OCC_CONVERT_SIGNALS

MSVC_VERSION = xx
win32-msvc2005:MSVC_VERSION = 8
win32-msvc2008:MSVC_VERSION = 9
win32-msvc2010:MSVC_VERSION = 10
win32-msvc2012:MSVC_VERSION = 11
win32-msvc2013:MSVC_VERSION = 12

# Find OCC version
OCC_VERSION_FILE_CONTENTS = $$cat($$CASCADE_ROOT/inc/Standard_Version.hxx, lines)

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

# Set CASCADE_SUB_LIB_PATH
equals(OCC_VERSION_STR, 6.7.1) \
    | equals(OCC_VERSION_STR, 6.8.0) \
    | equals(OCC_VERSION_STR, 6.9.0) \
    | equals(OCC_VERSION_STR, 6.9.1) \
    | equals(OCC_VERSION_STR, 7.0.0) \
{
  win32:CASCADE_SUB_LIB_PATH = win$${ARCH_BITS_SIZE}/vc$$MSVC_VERSION/lib
  linux-*:CASCADE_SUB_LIB_PATH = lin$${ARCH_BITS_SIZE}/gcc/lib
  CONFIG(debug, debug|release):CASCADE_SUB_LIB_PATH = $${CASCADE_SUB_LIB_PATH}d
} else {
  CASCADE_SUB_LIB_PATH = lib
}

CASCADE_LIB_PATH += $$CASCADE_ROOT/$$CASCADE_SUB_LIB_PATH
LIBS += $$sysPath($$join(CASCADE_LIB_PATH, " -L", -L))
QMAKE_RPATHDIR += $$CASCADE_LIB_PATH
