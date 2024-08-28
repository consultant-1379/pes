#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       cfeted.sh
# Description:
#       A script to wrap the invocation of cfeted from the COM CLI.
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
# - Wed Oct 3 2012 - uabmagn
#	First version.
##

Gid=$(/usr/bin/id -g)

if [[ $Gid == 0 ||  $Gid == 1003 ]];then
    /bin/logger 'cfeted execution started'
else
    if [[ $1 =~ '-bootOpt'|'-bootOptList' ]];then
       echo -e "ERROR: Not authorized to perform this operation"
       exit 1
    fi
fi

umask 002
/usr/bin/sudo /opt/ap/pes/bin/cfeted "$@"

exit $?
