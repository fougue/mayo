HEADERS += \
    $$files(../src/base/*.h) \
    $$files(../src/io_occ/*.h) \
    $$files(../src/io_dxf/*.h) \
    $$files(../src/io_gmio/*.h) \
    $$files(../src/graphics/*.h) \
    $$files(../src/gui/*.h) \
    $$files(../src/app/*.h) \
    $$files(../src/app/windows/*.h) \

SOURCES += \
    $$files(../src/base/*.cpp) \
    $$files(../src/io_occ/*.cpp) \
    $$files(../src/io_dxf/*.cpp) \
    $$files(../src/io_gmio/*.cpp) \
    $$files(../src/graphics/*.cpp) \
    $$files(../src/gui/*.cpp) \
    $$files(../src/app/*.cpp) \
    $$files(../src/app/windows/*.cpp) \
    \
    messages.cpp \

FORMS += $$files(../src/app/*.ui)

TRANSLATIONS += \
    mayo_en.ts \
    mayo_fr.ts
