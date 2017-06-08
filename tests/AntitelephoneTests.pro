TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += *.cpp
SOURCES += $$files(../src/*.cpp)
HEADERS += *.hpp

INCLUDEPATH += $$(VS2017BUILDTOOLS)/include
INCLUDEPATH += $$(BOOST_ROOT)
LIBS += "-L$$(BOOST_ROOT)/lib64-msvc-14.1"
DEFINES += "_WIN32_WINNT=0x0501"
