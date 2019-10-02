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
MAYO_VERSION_MIN = 1
MAYO_VERSION_PAT = 0
VERSION = \
    $${MAYO_VERSION_MAJ}.$${MAYO_VERSION_MIN}.$${MAYO_VERSION_PAT}.$${MAYO_VERSION_REVNUM}
MAYO_VERSION = $${VERSION}-$$MAYO_VERSION_COMMIT

# Generate version file
QMAKE_SUBSTITUTES += $$PWD/src/app/version.h.in

INCLUDEPATH += $$OUT_PWD/src  # To allow inclusion as "version.h" from source code
OTHER_FILES += $$PWD/src/app/version.h.in
