## Haiku Generic Makefile v2.6 ##

NAME = Space
TARGET_DIR = .
TYPE = SHARED
APP_MIME_SIG = application/x-vnd.SpaceScreenSaver

SRCS = Space.cpp SpaceConfigView.cpp
RDEFS = Space.rdef
RSRCS =

LIBS = be $(STDCPPLIBS)
LIBPATHS =

SYSTEM_INCLUDE_PATHS = \
	$(shell findpaths B_FIND_PATH_HEADERS_DIRECTORY)/os/add-ons/screen_saver
LOCAL_INCLUDE_PATHS =

OPTIMIZE = FULL
LOCALES =
DEFINES =
WARNINGS = ALL
SYMBOLS =
DEBUGGER =
COMPILER_FLAGS = -std=c++17
LINKER_FLAGS =
APP_VERSION =
DRIVER_PATH =

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
