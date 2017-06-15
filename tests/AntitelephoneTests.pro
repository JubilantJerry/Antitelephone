TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += *.cpp
SOURCES += $$files(../src/*.cpp)
HEADERS += *.hpp
HEADERS += $$files(../src/*.hpp)

TEST_FILES = "K:/Personal Projects/AntitelephoneQT/tests/test_files/"

INCLUDEPATH += $$(VS2017BUILDTOOLS)/include
INCLUDEPATH += $$(BOOST_ROOT)
INCLUDEPATH += ../src/pcg-cpp-0.98/include
LIBS += "-L$$(BOOST_ROOT)/lib64-msvc-14.1"
DEFINES += "_WIN32_WINNT=0x0501"
DEFINES += "PCG_LITTLE_ENDIAN=1"
DEFINES += "TEST_FILES_PATH=\"\\\"$$TEST_FILES\\\"\""
