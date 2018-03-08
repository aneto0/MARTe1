#############################################################
#
# Copyright 2011 EFDA | European Fusion Development Agreement
#
# Licensed under the EUPL, Version 1.1 or - as soon they 
# will be approved by the European Commission - subsequent  
# versions of the EUPL (the "Licence"); 
# You may not use this work except in compliance with the 
# Licence. 
# You may obtain a copy of the Licence at: 
#  
# http://ec.europa.eu/idabc/eupl
#
# Unless required by applicable law or agreed to in 
# writing, software distributed under the Licence is 
# distributed on an "AS IS" basis, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
# express or implied. 
# See the Licence for the specific language governing 
# permissions and limitations under the Licence. 
#
# $Id$
#
#############################################################
#!/bin/sh
module="pcieAdcMod"
device="pcieATCAAdc"
mode="666"

# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
#/sbin/insmod ./$module.ko $* || exit 1
/sbin/insmod ./$module.ko $* 
major=`cat /proc/devices | grep pcieATCAAdc | tail -n 1 | cut -d ' ' -f 1`
echo $major

# remove stale nodes
rm -f /dev/${device}[0-31]

mknod /dev/${device}0 c $major 0

chmod $mode  /dev/${device}*
ls -l /dev/${device}*
