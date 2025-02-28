HEADERS += \
    $$files(../src/app/*.h) \
    $$files(../src/app/windows/*.h) \
    $$files(../src/base/*.h) \
    $$files(../src/cli/*.h) \
    $$files(../src/graphics/*.h) \
    $$files(../src/gui/*.h) \
    $$files(../src/io_assimp/*.h) \
    $$files(../src/io_dxf/*.h) \
    $$files(../src/io_gmio/*.h) \    
    $$files(../src/io_image/*.h) \
    $$files(../src/io_occ/*.h) \
    $$files(../src/io_off/*.h) \
    $$files(../src/io_ply/*.h) \
    $$files(../src/measure/*.h) \


SOURCES += \
    $$files(../src/app/*.cpp) \
    $$files(../src/app/windows/*.cpp) \
    $$files(../src/base/*.cpp) \
    $$files(../src/cli/*.cpp) \
    $$files(../src/graphics/*.cpp) \
    $$files(../src/gui/*.cpp) \
    $$files(../src/io_assimp/*.cpp) \
    $$files(../src/io_dxf/*.cpp) \
    $$files(../src/io_gmio/*.cpp) \
    $$files(../src/io_image/*.cpp) \
    $$files(../src/io_occ/*.cpp) \
    $$files(../src/io_off/*.cpp) \
    $$files(../src/measure/*.cpp) \
    \
    messages.cpp \

FORMS += $$files(../src/app/*.ui)

TRANSLATIONS += \
    mayo_en.ts \
    mayo_fr.ts \
    mayo_zh.ts \
