#-------------------------------------------------
#
# Project created by QtCreator 2016-05-20T10:29:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = r8asm
TEMPLATE = app


SOURCES += main.cpp\
        r8asmwindow.cpp \
    r8engine.cpp \
    r8compiler.cpp \
    r8charstream.cpp \
    r8sourceeditor.cpp \
    r8syntaxhighlighter.cpp \
    r8lexer.cpp \
    r8inputdialog.cpp

HEADERS  += r8asmwindow.h \
    r8engine.h \
    r8compiler.h \
    r8charstream.h \
    r8sourceeditor.h \
    r8syntaxhighlighter.h \
    r8lexer.h \
    r8inputdialog.h

FORMS    += r8asmwindow.ui \
    r8inputdialog.ui

RESOURCES += \
    r8resource.qrc

TRANSLATIONS += \
    translate/r8asm_ru.tr
