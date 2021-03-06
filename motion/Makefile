
CUR_DIR = $(shell pwd)
BUILD_DIR = build
INSTALL_DIR = ../../install
CMAKE_DIR = cmake.man.motion
CMAKE_DEPEND = $(BUILD_DIR)/Makefile
CMAKE_SRCS = \
	CMakeLists.txt \
	buildconfig.cmake \
	motionconfig.h
CMAKE_SRCS := $(addprefix $(CMAKE_DIR),$(CMAKE_SRCS))
CONFIG_FILE = motionconfig.h
CROSS_FILE = $(BUILD_DIR)/cross
STRAIGHT_FILE = $(BUILD_DIR)/straight

CROSS_TOOLCHAIN = $(abspath $(CURDIR)/../../cmake/geode.cmake)
CMAKE_DEFS = -DOE_CROSS_BUILD=OFF
CMAKE_CROSS_DEFS = -DCMAKE_TOOLCHAIN_FILE=$(CROSS_TOOLCHAIN) -DOE_CROSS_BUILD=ON

TRUNK_REVISION = r0 #$(shell svn info | grep Revision | awk 'FS=" " {print $$2}')
export TRUNK_REVISION

CD=cd
CMAKE=cmake
CCMAKE=ccmake
MKDIR=mkdir
TOUCH=touch
MAKE_OPTIONS=--no-print-directory


.PHONY: default_target all straight cross cclean clean install noconfig
.PHONY: clean_install


default_target: all

all: $(CMAKE_DEPEND)
	@#echo $(TRUNK_REVISION)
	@$(MAKE) $(MAKE_OPTIONS) -C $(BUILD_DIR)

straight: $(STRAIGHT_FILE)
	@set -e; \
		$(CD) $(BUILD_DIR); \
		$(CCMAKE) .

$(CMAKE_DEPEND): 
	@if [ ! -e $(STRAIGHT_FILE) ]; then \
		$(MAKE) $(MAKE_OPTIONS) cross all; \
	else \
		$(MAKE) $(MAKE_OPTIONS) straight all; \
	fi

$(STRAIGHT_FILE):
	@if [ -e $(CROSS_FILE) ]; then \
		$(MAKE) $(MAKE_OPTIONS) clean; \
	fi
	@set -e; \
		$(MKDIR) -p $(BUILD_DIR); \
		$(TOUCH) $(STRAIGHT_FILE); \
		$(CD) $(BUILD_DIR); \
		$(CMAKE) $(CMAKE_DEFS) $(CURDIR)/$(CMAKE_DIR)

cross: $(CROSS_FILE)
	@set -e ; \
		$(CD) $(BUILD_DIR); \
		$(CCMAKE) .

$(CROSS_FILE):
	@if [ -e $(STRAIGHT_FILE) ]; then \
	       $(MAKE) clean; \
        fi
	@set -e; \
		$(MKDIR) -p $(BUILD_DIR); \
		$(TOUCH) $(CROSS_FILE); \
		$(CD) $(BUILD_DIR); \
		$(CMAKE) $(CMAKE_CROSS_DEFS) $(CURDIR)/$(CMAKE_DIR)

install: $(CMAKE_DEPEND)
	@$(MAKE) $(MAKE_OPTIONS) -C $(BUILD_DIR) install

clean_install:
	$(RM) -r $(INSTALL_DIR)/*

cclean: $(CMAKE_DEPEND)
	@$(MAKE) $(MAKE_OPTIONS) -C $(BUILD_DIR) clean

clean:
	$(RM) -r $(BUILD_DIR) $(CONFIG_FILE)


