# **********************************************************************
#
# Short description:
# Makefile for pes_clh_server at CAA level
# **********************************************************************
#
# Ericsson AB 2013 All rights reserved.
# The information in this document is the property of Ericsson.
# Except as specifically authorized in writing by Ericsson, the
# receiver of this document shall keep the information contained herein
# confidential and shall protect the same in whole or in part from
# disclosure and dissemination to third parties. Disclosure and 
# disseminations to the receivers employees shall only be made  on a 
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
CNZ_NAME ?= clh_cnz
CAA_NAME ?= clhadm_caa
CXC_NAME ?= clhbin_cxc

# Basic paths
VOB_PATH = $(PES_ROOT)
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

BLOCKDIR = $(CAA_PATH)
OUTDIR   = $(CXC_PATH)/bin
OBJDIR	 = $(BLOCKDIR)/obj
SRCDIR	 = $(BLOCKDIR)/src
INCDIR	 = $(BLOCKDIR)/inc
TESTDIR  = $(BLOCKDIR)/test
CLHINCDIR = $(BLOCKDIR)/../clhlib_caa/inc
CLHLIBDIR = $(OUTDIR)/lib_int
PRODUCT = 19089-CAA1091416
BASEDIR = `basename $(CURDIR)`

# Flexelint application
FL    = $(LINT)     # Global wrap-up mode
FL1   = $(LINT) -u  # Single unit mode

# Here you can add own compiler flags
#CPPFLAGS += -g -O2 -Wall 
CFLAGS += -DNDEBUG
CFLAGS += -DBOOST_FILESYSTEM_VERSION=3
CFLAGS += -DBOOST_FILESYSTEM_NO_DEPRECATED
CFLAGS += -DMULTI_CP_SYSTEM

LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_LOGCPLUS_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_XERCES_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(BOOST_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(ACE_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(COREMW_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB)

# Here you can add own lib paths
# This may need to be modified later once external libraries are available 
LIBSDIR += -L$(CLHLIBDIR) -L$(AP_SDK_LIB) -L$(BOOST_SDK_LIB) -L$(COREMW_SDK_LIB)

# Here you can add own Assembler flags
ASMFLAGS +=

## Here you can add own Include paths and/or other includes
# This may need to be modified later once external libraries are available
CINCLUDES += -I$(INCDIR) -I$(CLHINCDIR) -I$(BOOST_SDK_INC) -I$(ACE_SDK_INC) -I$(COREMW_SDK_INC)

## Here you can add own libs
# This may need to be modified later once external libraries are available 
LIBS += -lpes_clh -lacs_csapi -lacs_apgcc -lacs_apbm -lboost_regex -lboost_thread -lboost_filesystem \
-lSaImmOm -lacs_tra -lboost_system

## Here you can add own File paths
VPATH += $(SRCDIR) $(INCDIR) $(OUTDIR) $(OBJDIR)

## Source files and Object files

L_FILES += $(SRCFILES:%=$(BLOCKDIR)/src/%) 

SRCFILES = $(wildcard $(SRCDIR)/*.cpp)

SRCTOOBJ =  \
        $(patsubst %.cpp, %.obj, $(SRCFILES))
OBJMOD = $(subst src,obj,$(SRCTOOBJ))
ADM_OBJ = $(subst $(OBJDIR)/, ,$(OBJMOD))

## Build instructions

#.cpp.obj:
#	$(NEW_LINE)
#	$(SEPARATOR_STR)
#	$(SILENT)$(ECHO) 'Compiling file: $<'
#	$(CC) $(EXTRA_CFLAGS) $(GCOV_FLAGS) -c $(CFLAGS)  $(CINCLUDES) -I$(AP_SDK_INC) $< -o $(OBJDIR)/$(@F)
#	$(SEPARATOR_STR)

ADM_APNAME = pes_clhd
ADM_APEXE = $(OUTDIR)/$(ADM_APNAME)

.PHONY: all
all: $(ADM_APEXE)

$(OUTDIR)/$(ADM_APNAME): $(ADM_OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) "Linking $@..."
	$(SILENT)$(CC) -o $@ $(OBJMOD) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(call stripp,$(ADM_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE) 

.PHONY: clean
clean:
	$(SILENT)$(ECHO) 'Cleaning files'
	$(SILENT)$(RM) $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(ADM_APNAME)

