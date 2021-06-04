system(git --version):HAVE_GIT=1
defined(HAVE_GIT, var) {
    MAYO_VERSION_COMMIT = $$system(git rev-parse --short HEAD)
    MAYO_VERSION_REVNUM = $$system(git rev-list --count HEAD)
} else {
    MAYO_VERSION_COMMIT = ??
    MAYO_VERSION_REVNUM = 0
    warning("Git is not in PATH, cannot find commit ID nor revision number")
}

MAYO_VERSION_MAJ = 0
MAYO_VERSION_MIN = 4
MAYO_VERSION_PAT = 0
VERSION = $${MAYO_VERSION_MAJ}.$${MAYO_VERSION_MIN}.$${MAYO_VERSION_PAT}.$${MAYO_VERSION_REVNUM}
MAYO_VERSION = $${VERSION}-$$MAYO_VERSION_COMMIT

equals(QT_ARCH, i386) {
    VERSION_TARGET_ARCH = x86
} else:equals(QT_ARCH, x86_64) {
    VERSION_TARGET_ARCH = x64
} else {
    VERSION_TARGET_ARCH = $$QT_ARCH
}

QMAKE_TARGET_PRODUCT = Mayo
QMAKE_TARGET_COMPANY = Fougue

# Generate version files
QMAKE_SUBSTITUTES += \
    $$PWD/installer/version.iss.in \
    $$PWD/src/app/version.h.in

INCLUDEPATH += $$OUT_PWD/src/app  # To allow inclusion as "version.h" from source code
OTHER_FILES += $$PWD/src/app/version.h.in
