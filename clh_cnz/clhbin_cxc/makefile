# **********************************************************************
#
# Short description:
# Makefile for clhbin at CXC level
# **********************************************************************
#
# Ericsson AB 2013 All rights reserved.
# The information in this document is the property of Ericsson.
# Except as specifically authorized in writing by Ericsson, the receiver
# of this document shall keep the information contained herein
# confidential and shall protect the same in whole or in part from
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
# **********************************************************************
# BEGIN: GIT INTRO.
CURDIR = $(shell pwd)
REPO_NAME = pes
PES_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(PES_ROOT)/common

include $(COMMON_ROOT)/common.mk 

CXCPATH = $(CURDIR)
CXCDIR = $(CXCPATH)

CXC_NR ?= CXC1372167_9
CXC_VER ?= R1K
#CXC_OLD_VER ?= 'R1A05|R1A04|R1A03|R1A02|R1A01|R1A02'
CXC_NAME ?= PES_CLHBIN
BLOCK_NAME = $(CXC_NAME)
BASE_SW_VER ?= 3.6.0-R1A 
PRODUCT = 19089-$(CXC_NR)
CXC_BLK_NAME ?= pes_clh
CAA_DIRS =  \
	$(CXCDIR)/../clhlib_caa \
	$(CXCDIR)/../clhcmd_caa \
	$(CXCDIR)/../clhadm_caa \
	$(CXCDIR)/../clhapi_caa

FX_CLEAN_TMP := rm -rf /tmp/$(USER)/*

.PHONY: all
all:
	@for i in $(CAA_DIRS) ; do \
	(echo "+++++++++++++++++" ; \
	echo "cd $$i" ; \
	cd $$i ; \
	$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) PES_ROOT=$(PES_ROOT) all) ;\
	if [ $$? != 0 ];\
	then  $(ECHO) "Error:Please see the printout!!!";\
	exit $$?;\
	fi;\
	echo "+++++++++++++++++" ; \
	done

# !!! "make release" shouldn't be executed since the rpm creation buildscript might need a change ??check 

.PHONY: release
release: all rpm_package sdp_package esm_package tar_package ap_sdk documentation metrics

.PHONY: nightbuild
nightbuild: del_update distclean all rpm_package sdp_package esm_package documentation metrics 

.PHONY: del_update
del_update:
	$(call rpm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call sdp_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call esm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_ESM_VER),$(CXC_NR))

.PHONY: rpm_package
rpm_package:
	$(call rpm_file_creation,clh.spec,$(CXC_NAME),$(CXC_NR),$(CXC_VER))

.PHONY: sdp_package
sdp_package:
	$(SILENT)$(ECHO) 'SDP packaging ...'
	$(call sdp_creation_US3,$(CXCPATH),$(CXC_NAME),$(CXC_VER),$(CXC_OLD_VER),$(CXC_NR))
	$(call up_creation,$(CXC_NAME),$(CXC_NR),$(CXC_VER),$(CXC_OLD_VER),$(CXCPATH),$(BASE_SW_VER))

.PHONY: esm_package
esm_package:
	$(call esm_pkg_creation,$(CXCPATH),$(CXC_BLK_NAME),$(CXC_VER),$(BASE_SW_VER),$(CXC_NR),$(CXC_NAME))
                      
.PHONY: upload_package 
upload_package: 
	$(call upload_package,$(CXC_BLK_NAME),$(CXC_NR),$(CXC_NAME),$(CXC_VER))

.PHONY: tar_package
tar_package:
	$(SILENT)$(ECHO) 'TAR packaging ... => DO NOTHING...'
#	$(call tar_creation,$(BLOCK_NAME),$(CXC_NR),$(CXC_VER))

.PHONY: documentation
documentation:
	$(SILENT)$(ECHO) ' Removing ./docgen directory'
	$(SILENT)$(RM) -r -f ./docgen 
	$(SILENT)$(ECHO) ' Creating ./docgen directory'
	$(SILENT)mkdir ./docgen 
	$(SILENT)$(DOXYGEN) $(DOXYGENFILECONFIG) 

.PHONY: metrics
metrics:
	@for i in $(CAA_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		 echo "cd $$i" ; \
		cd $$i ; \
		$(MAKE) $(MFLAGS) metrics ) ;\
		echo "+++++++++++++++++" ; \
	done	

.PHONY: clean
clean:
	@for i in $(CAA_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		 echo "cd $$i" ; \
		 cd $$i ; \
		 $(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) PES_ROOT=$(PES_ROOT) clean ) ;\
		 echo "+++++++++++++++++" ; \
	done
	$(call FX_CLEAN_TMP)

.PHONY: distclean
distclean:
	@for i in $(CAA_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		 echo "cd $$i" ; \
		 cd $$i ; \
		 $(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) PES_ROOT=$(PES_ROOT) distclean ) ;\
		 echo "+++++++++++++++++" ; \
	done
	$(call FX_CLEAN_TMP)

.PHONY: ap_sdk
ap_sdk:
	$(SILENT)$(ECHO) 'AP SDK updating ...'
	$(SILENT)$(ECHO) '**********************************'
	$(SILENT)$(ECHO) '****** NOT YET IMPLEMENTED *******'
	$(SILENT)$(ECHO) '**********************************'

.PHONY: help
help:
	$(SILENT)$(ECHO) 'Cleaning targets'\
	$(SILENT)$(ECHO) '  clean         - Removes intermediary files generated during the compilation'\
	$(SILENT)$(ECHO) '  distclean     - Removes anything generated during the compilation, to start allow a fresh start'\
	$(SILENT)$(ECHO) 'Other generic targets'\
	$(SILENT)$(ECHO) '  all           - Build all targets'\
	$(SILENT)$(ECHO) '  release       - Build all targets and create rpm, sdp, ...'\
	$(SILENT)$(ECHO) '  Execute "make" or "make all" to build all targets '\
	$(SILENT)$(ECHO) '  make BUILD_VERBOSE=0|1 [targets] 0 => quiet build (default), 1 => verbose build'\
	$(SILENT)$(ECHO) '  make COVERAGE=0|1 [targets] 0 => no coverage (default), 1 => coverage options'\
	$(SILENT)$(ECHO) '  make LINT=0|1   [targets] Check all c source with $$CHECK )'\

.PHONY: print_build_envs
print_build_envs:
	$(SILENT)$(ECHO) ' Build Verbose is $(V)'
	$(SILENT)$(ECHO) ' Build Verbose is $(SILENT)'
	$(SILENT)$(ECHO) ' Build Verbose is $(quiet)'
	$(SILENT)$(SEPARATOR_STR)
	$(SILENT)$(ECHO) $(AS)
	$(SILENT)$(ECHO) $(LD)
	$(SILENT)$(ECHO) $(CC)
	$(SILENT)$(ECHO) $(CPP)
	$(SILENT)$(ECHO) $(NM)
	$(SILENT)$(ECHO) $(STRIP)
	$(SILENT)$(ECHO) $(OBJCOPY)
	$(SILENT)$(ECHO) $(OBJDUMP)
	$(SILENT)$(ECHO) $(AWK)
	$(SILENT)$(ECHO) $(PERL)
	$(SILENT)$(ECHO) $(AS)
	$(SILENT)$(ECHO) $(RM)
	$(SILENT)$(ECHO) $(TOUCH)
	$(SILENT)$(ECHO) $(CHECK)
	$(SILENT)$(ECHO) $(LINT)
	$(SILENT)$(ECHO) $(DOXYGEN)
	$(SILENT)$(ECHO) $(CCCC)
	$(SILENT)$(ECHO) $(SLOCCOUNT)
	$(SILENT)$(ECHO) $(CONFIG_SHELL)
	$(SILENT)$(ECHO) $(PWD)
	$(SILENT)$(SEPARATOR_STR)

.PHONY: debug
debug: 	print_build_envs
	$(SILENT)$(ECHO) $(srctree)
	$(SILENT)$(ECHO) $(objtree)
	$(SILENT)$(ECHO) $(src)
	$(SILENT)$(ECHO) $(obj)
	$(SILENT)$(ECHO) $(VPATH)
	$(SILENT)$(ECHO) $(CURDIR)

.PHONY: archive
archive:
	@for i in $(CAA_DIRS) ; do \
		cd $$i ; \
		$(MAKE) archive ;\
	done

