VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0

system(git --version):HAVE_GIT=1
defined(HAVE_GIT, var) {
    MAYO_REVNUM = $$system(git rev-parse --short HEAD)
} else {
    warning("Git is not in PATH, cannot find revision number")
}
MAYO_VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}-r$$MAYO_REVNUM

# Generate version file
QMAKE_SUBSTITUTES += $$PWD/src/version.h.in

CONFIG(debug, debug|release) {
    message(Version $$MAYO_VERSION debug)
} else {
    message(Version $$MAYO_VERSION release)
}

INCLUDEPATH += $$OUT_PWD/src  # To allow inclusion as "version.h" from source code
