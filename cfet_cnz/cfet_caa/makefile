# **********************************************************************
#
# Short description:
# Makefile for cfeted at CAA level
# **********************************************************************
#
# Ericsson AB 2010 All rights reserved.
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
# -----      -------      --------  --------------------------
#            2010-12-08   xcsvere   Created
#            2012-04-23   uabmagn   debug symbols created $(call stripp, ...)
#            2016-01-21   egiaava   Adapted for SLES12 and new BOOST LIB 1_54
#
#***********************************************************************
CURDIR = $(shell pwd)
REPO_NAME = pes
PES_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(PES_ROOT)/common
 
include $(COMMON_ROOT)/common.mk

CXCDIR = $(PES_ROOT)/cfet_cnz/cfet_cxc
#BLOCKDIR = $(CURDIR)
#OUTDIR   = $(CXCDIR)/bin
#OBJDIR	 = $(BLOCKDIR)/obj
#SRCDIR	 = $(BLOCKDIR)/src
#INCDIR	 = $(BLOCKDIR)/inc
#TESTDIR  = $(BLOCKDIR)/test

# Flexelint application
FL    = $(LINT)     # Global wrap-up mode
FL1   = $(LINT) -u  # Single unit mode

# Here you can add own compiler flags
#CPPFLAGS += -g -O2 -Wall
#CFLAGS += -fprofile-arcs -ftest-coverage 

## Here you can add own Include paths and/or other includes
# This may need to be modified later once external libraries are available

##### --- CINCLUDES += -I$(CURDIR)/CSAPI/inc

CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(LOTC_SDK_INC)\
-I$(AP_SDK_INC) -I$(COREMW_SDK_INC) -I$(ACE_SDK_INC)
# For AP MAUS API
#CINCLUDES += -I$(CURDIR)/APMAUSAPI/inc

# Here you can add own lib paths
# This may need to be modified later once external libraries are available 
LIBSDIR  = -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LOTC_SDK_LIB) -L$(COREMW_SDK_LIB) -L$(ACE_SDK_LIB)
# For AP MAUS API
#LIBSDIR += -L$(CURDIR)/APMAUSAPI/lib

#For ACS CS API
##### --- LIBSDIR += -L$(CURDIR)/CSAPI/lib

# Here you can add own Assembler flags
ASMFLAGS += 

#CFLAGS += -g -Wall -ansi -DNDEBUG -std=c++0x
LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(COREMW_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_LOGCPLUS_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_XERCES_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(BOOST_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(ACE_SDK_LIB)

## Here you can add own libs
# This may need to be modified later once external libraries are available 
LIBS += -lacs_csapi -lboost_filesystem -lboost_regex -lboost_system -lacs_apgcc -lacs_prc -lboost_program_options

LIBS += $(COREMW_LIBS)
#
# LIBS += -lgcov
# For AP MAUS API
LIBS += -lcphw_mauslibh

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
#	    $(NEW_LINE)
#	    $(SEPARATOR_STR)
#	    $(SILENT)$(ECHO) 'Compiling file: $<'
#	    $(CC) $(EXTRA_CFLAGS) $(GCOV_FLAGS) -c $(CFLAGS)  $(CINCLUDES) -I$(AP_SDK_INC) $< -o $(OBJDIR)/$(@F)
#	    $(SEPARATOR_STR)

CMD_APNAME = cfeted
CMD_APEXE = $(OUTDIR)/$(CMD_APNAME)

.PHONY: all
all: $(CMD_APEXE)

$(OUTDIR)/$(CMD_APNAME): $(CMD_OBJ)
	    $(NEW_LINE)
	    $(SEPARATOR_STR)		    		    	
	    $(SILENT)$(ECHO) "Linking $@..."
	    $(CC) -o $@ $(OBJMOD) $(LDFLAGS) $(LIBSDIR) $(LIBS)
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

