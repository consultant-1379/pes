#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       clh.sh
# Description:
#       A script to wrap the invocation of clh commands from the COM CLI.
# Note:
#	None.
##
# Usage:
#	None.
##
# Output:
#       None.
##
# Changelog:
#	2012-10-04 - Thomas Olsson (uabtso)
#	First version.
##

umask 002
/usr/bin/sudo /opt/ap/pes/bin/`basename "$0"` "$@"

exit $?
