# **********************************************************************
#
# Short description:
# Makefile for command sells in CAA level
# **********************************************************************
#
# Ericsson AB 2013 All rights reserved.
# The information in this document is the property of Ericsson.
# Except as specifically authorized in writing by Ericsson, the receiver of this
# document shall keep the information contained herein confidential and shall protect
# the same in whole or in part from disclosure and dissemination to third parties.
# Disclosure and disseminations to the receivers employees shall only be made
# on a strict need to know basis.
#
# **********************************************************************
#
# Rev        Date         Name      What
# ---        ----         ----      ----
# R2A        2013-03-15   UABTSO    Created
# R3A        2016-01-29   EGIAAVA   Updated for SLES12 and BOOST_154
#
#***********************************************************************
CURDIR = $(shell pwd)
REPO_NAME = pes
PES_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(PES_ROOT)/common
 
include $(COMMON_ROOT)/common.mk 

CXCDIR = $(CURDIR)/../../clhbin_cxc
BLOCKDIR = $(CURDIR)
OUTDIR   = $(CXCDIR)/bin
OBJDIR	 = $(BLOCKDIR)/obj
SRCDIR	 = $(BLOCKDIR)/src
INCDIR	 = $(BLOCKDIR)/inc
TESTDIR  = $(BLOCKDIR)/test
CLHINCDIR = $(BLOCKDIR)/../../clhlib_caa/inc
CLHLIBDIR = $(OUTDIR)/lib_int

# Flexelint application
FL    = $(LINT)     # Global wrap-up mode
FL1   = $(LINT) -u  # Single unit mode

# Here you can add own compiler flags
#CPPFLAGS += -g -O2 -Wall 
CFLAGS += -DNDEBUG
CFLAGS += -DBOOST_FILESYSTEM_VERSION=3
CFLAGS += -DBOOST_FILESYSTEM_NO_DEPRECATED

# Here you can add own lib paths
# This may need to be modified later once external libraries are available 
LIBSDIR += -L$(CLHLIBDIR) -L$(AP_SDK_LIB) -L$(BOOST_SDK_LIB)

# Here you can add own Assembler flags
ASMFLAGS +=

## Here you can add own Include paths and/or other includes
# This may need to be modified later once external libraries are available
CINCLUDES += -I$(INCDIR) -I$(CLHINCDIR) -I$(BOOST_SDK_INC) -I$(ACE_SDK_INC)

LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_LOGCPLUS_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_XERCES_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(BOOST_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(ACE_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(COREMW_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB)

## Here you can add own libs
# This may need to be modified later once external libraries are available 
LIBS += -lpes_clh -lacs_apgcc -lboost_filesystem -lboost_system

## Here you can add own File paths
VPATH += $(SRCDIR) $(INCDIR) $(OUTDIR) $(OBJDIR)

## Source files and Object files

L_FILES += $(SRCFILES:%=$(BLOCKDIR)/src/%) 

SRCFILES = $(wildcard $(SRCDIR)/*.cpp)

SRCTOOBJ =  \
        $(patsubst %.cpp, %.obj, $(SRCFILES))
OBJMOD = $(subst src,obj,$(SRCTOOBJ))
CMD_OBJ = $(subst $(OBJDIR)/, ,$(OBJMOD))

## Build instructions

#.cpp.obj:
#	$(NEW_LINE)
#	$(SEPARATOR_STR)
#	$(SILENT)$(ECHO) 'Compiling file: $<'
#	$(CC) $(EXTRA_CFLAGS) $(GCOV_FLAGS) -c $(CFLAGS)  $(CINCLUDES) -I$(AP_SDK_INC) $< -o $(OBJDIR)/$(@F)
#	$(SEPARATOR_STR)

CMD_APNAME = sells
CMD_APEXE = $(OUTDIR)/$(CMD_APNAME)

.PHONY: all
all: $(CMD_APEXE)

$(OUTDIR)/$(CMD_APNAME): $(CMD_OBJ)
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) "Linking $@..."
	$(SILENT)$(CC) -o $@ $(OBJMOD) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(call stripp,$(CMD_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)

.PHONY: clean
clean:
	$(SILENT)$(ECHO) 'Cleaning files'
	$(SILENT)$(RM) $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(CMD_APNAME)

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

# DO NOT DELETE THIS LINE -- make depend needs it

