# **********************************************************************
#
# Short description:
# Makefile for clhcmd at CAA level
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
# -----      ----         ----      ----
# R2A        2013-01-09   UABTSO    Created
# R2A04      2013-03-01   XVUNGUY   Append rpls + rptran command
#
# **********************************************************************

CURDIR = $(shell pwd)
CAAPATH = $(CURDIR)
CAADIR = $(CAAPATH)
PRODUCT = 19089-CAA1091425
BASEDIR = `basename $(CURDIR)`/`basename $$i`

CMD_DIRS =  \
            $(CAADIR)/clhls \
            $(CAADIR)/clhtran \
            $(CAADIR)/sells \
            $(CAADIR)/rpls \
            $(CAADIR)/rptran \
            $(CAADIR)/seltran

.PHONY: all
all:
	@for i in $(CMD_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		echo "cd $$i" ; \
		cd $$i ; \
		$(MAKE) $(MFLAGS) CAADIR=$(CAAPATH) all) ;\
		echo "+++++++++++++++++" ; \
	done

.PHONY: clean
clean:
	@for i in $(CMD_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		echo "cd $$i" ; \
		cd $$i ; \
		$(MAKE) $(MFLAGS) CAADIR=$(CAAPATH) clean ) ;\
		echo "+++++++++++++++++" ; \
	done

.PHONY: distclean
distclean:
	@for i in $(CMD_DIRS) ; do \
		(echo "+++++++++++++++++" ; \
		echo "cd $$i" ; \
		cd $$i ; \
		$(MAKE) $(MFLAGS) CAADIR=$(CAAPATH) distclean ) ;\
		echo "+++++++++++++++++" ; \
	done

.PHONY: archive
archive:
	$(SILENT)$(ECHO) Creating archive $(PRODUCT).tar
	$(SILENT)tar --create --file=../$(PRODUCT).tar --files-from=/dev/null
	@for i in $(CMD_DIRS) ; do \
	  tar --update --directory=../ --file=../$(PRODUCT).tar $(BASEDIR)/src $(BASEDIR)/inc $(BASEDIR)/makefile; \
 	done

