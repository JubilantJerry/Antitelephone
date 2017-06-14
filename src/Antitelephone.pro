TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += ../main.cpp
SOURCES += *.cpp
HEADERS += *.hpp

INCLUDEPATH += $$(VS2017BUILDTOOLS)/include
INCLUDEPATH += $$(BOOST_ROOT)
INCLUDEPATH += pcg-cpp-0.98/include
LIBS += "-L$$(BOOST_ROOT)/lib64-msvc-14.1"
DEFINES += "_WIN32_WINNT=0x0501"
DEFINES += "PCG_LITTLE_ENDIAN=1"
