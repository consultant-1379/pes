# **********************************************************************
#
# Short description:
# Makefile for clhlib (libpes_clh.so) at CAA level
# **********************************************************************
#
# Ericsson AB 2013 All rights reserved.
# The information in this document is the property of Ericsson.
# Except as specifically authorized in writing by Ericsson, the receiver
#  of this document shall keep the information contained herein
#  confidential and shall protect the same in whole or in part from
# disclosure and dissemination to third parties. Disclosure and
# disseminations to the receivers employees shall only be made on a 
# strict need to know basis.
#
# **********************************************************************
#
# Rev        Date         Name      What
# ---        ----         ----      ----
# R2A        2013-03-15   UABTSO    Created
# R3A        2016-01-29   EGIAAVA   Updated for SLES12 and BOOST 1_54
#
#***********************************************************************
CURDIR = $(shell pwd)
REPO_NAME = pes
PES_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(PES_ROOT)/common
 
include $(COMMON_ROOT)/common.mk

BLOCKDIR = $(CURDIR)
OUTDIR   = $(CURDIR)/../clhbin_cxc/bin/lib_int
OBJDIR	 = $(BLOCKDIR)/obj
SRCDIR	 = $(BLOCKDIR)/src
INCDIR	 = $(BLOCKDIR)/inc
TESTDIR  = $(BLOCKDIR)/test
PRODUCT = 19089-CAA1091424
BASEDIR = `basename $(CURDIR)`

# Flexelint application
FL    = $(LINT)     # Global wrap-up mode
FL1   = $(LINT) -u  # Single unit mode

# Here you can add own compiler flags
#CPPFLAGS += -g -O2 -Wall 
CFLAGS += -DNDEBUG
CFLAGS += -DBOOST_FILESYSTEM_VERSION=3
CFLAGS += -DBOOST_FILESYSTEM_NO_DEPRECATED

CFLAGS += -g -Wall -ansi -DNDEBUG -Wno-unused-local-typedefs -fpermissive
#CFLAGS += -g -Wall -ansi -DNDEBUG -Wunused-local-typedefs -fpermissive

# Here you can add own lib paths
# This may need to be modified later once external libraries are available 
LIBSDIR += -L$(AP_SDK_LIB) -L$(BOOST_SDK_LIB)
LIBSDIR += -L$(CURDIR)/CSAPI/lib

# Here you can add own Assembler flags
ASMFLAGS += 

## Here you can add own Include paths and/or other includes
# This may need to be modified later once external libraries are available
CINCLUDES += -I$(INCDIR) -I$(LOTC_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH) \
	-I$(ACE_SDK_INC) -I$(COREMW_SDK_INC) -I$(BOOST_SDK_INC)
CINCLUDES += -I$(CURDIR)/CSAPI/inc

## Here you can add own libs
# This may need to be modified later once external libraries are available 
LIBS = -lacs_csapi -lacs_apgcc -lacs_apbm -lboost_filesystem -lboost_regex -lboost_thread

## Here you can add own File paths
VPATH += $(SRCDIR) $(INCDIR) $(OUTDIR) $(OBJDIR)

## Source files and Object files

L_FILES += $(SRCFILES:%=$(BLOCKDIR)/src/%) 

SRCFILES = $(wildcard $(SRCDIR)/*.cpp)

SRCTOOBJ =  \
        $(patsubst %.cpp, %.obj, $(SRCFILES))
OBJMOD = $(subst src,obj,$(SRCTOOBJ))
LIB_OBJ = $(subst $(OBJDIR)/, ,$(OBJMOD))

## Build instructions

#.cpp.obj:
#	$(NEW_LINE)
#	$(SEPARATOR_STR)
#	$(SILENT)$(ECHO) 'Compiling file: $<'
#	$(CC) $(EXTRA_CFLAGS) $(GCOV_FLAGS) -c $(CFLAGS)  $(CINCLUDES) -I$(AP_SDK_INC) $< -o $(OBJDIR)/$(@F)
#	$(SEPARATOR_STR)

LIB_NAME = pes_clh
LIB_REL_MAJOR ?= 1
LIB_REL_MINOR ?= 0
LIB_REL_BUILD ?= 0
LIB_VER ?= $(LIB_REL_MAJOR).$(LIB_REL_MINOR).$(LIB_REL_BUILD)
LIB_FILENAME = lib$(LIB_NAME).so
LIB_SONAME = lib$(LIB_NAME).so.$(LIB_REL_MAJOR)
LIB_LINKER_NAME = lib$(LIB_NAME).so

$(LIB_NAME)_APLIB = $(OUTDIR)/$(LIB_FILENAME)
$(LIB_NAME)_APLIB_SO = $(OUTDIR)/$(LIB_LINKER_NAME)

.PHONY: all
all: $(OUTDIR)/$(LIB_FILENAME)

$(OUTDIR)/$(LIB_FILENAME): $(LIB_OBJ)
	$(SILENT)$(ECHO) 'Creating library: $(LIB_FILENAME)'
	$(SILENT) $(CC) -shared -o $($(LIB_NAME)_APLIB_SO) $(OBJMOD) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(call stripp,$(LIB_FILENAME))
	$(NEW_LINE)

.PHONY: clean
clean:
	$(SILENT)$(ECHO) 'Cleaning files'
	$(SILENT)$(RM) $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(LIB_FILENAME)

.PHONY: documentation
documentation:
	$(SILENT)$(ECHO) 'documentation ...'
	$(SILENT)$(ECHO) '**********************************'
	$(SILENT)$(ECHO) '****** NOT YET IMPLEMENTED *******'
	$(SILENT)$(ECHO) '**********************************'

.PHONY: metrics
metrics:
	$(SILENT)$(ECHO) 'metrics ...'
	$(SILENT)$(ECHO) '**********************************'
	$(SILENT)$(ECHO) '****** NOT YET IMPLEMENTED *******'
	$(SILENT)$(ECHO) '**********************************'

cccc:
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'C/C++ Code Counter file: $@'
	$(CCCC) $(wildcard $(BLOCKDIR)/src/*)
	$(SEPARATOR_STR)

lint:
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'LINT file: $@'
	$(LINT) $(wildcard $(BLOCKDIR)/src/*)
	$(SEPARATOR_STR)

.PHONY: depend
depend: $(SRCFILES)
	makedepend $(CINCLUDES) $^

.PHONY: archive
archive:
	$(SILENT)$(ECHO) Creating archive $(PRODUCT).tar
	$(SILENT)tar --create --directory=../ --file=../$(PRODUCT).tar $(BASEDIR)/src $(BASEDIR)/inc $(BASEDIR)/makefile

# DO NOT DELETE THIS LINE -- make depend needs it

