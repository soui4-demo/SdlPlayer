TEMPLATE = subdirs
TARGET = SdlPlayer
CONFIG(x64){
TARGET = $$TARGET"64"
}
DEPENDPATH += .
INCLUDEPATH += .

include(cpy-cfg.pri)

SUBDIRS += SDL2
SUBDIRS += SdlPlayer

SdlPlayer.depends += SDL2
