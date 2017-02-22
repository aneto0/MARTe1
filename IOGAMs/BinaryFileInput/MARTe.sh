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
#Start-up script for the MARTe example
#!/bin/sh 

if [ -z "$1" ]; then
	echo "Please specify the location of the configuration file"
	exit
else
	echo "Going to start MARTe with the configuration specified in: " $1
fi

target=`uname`
case ${target} in
    Darwin)
    TARGET=macosx
    ;;
    SunOS)
    TARGET=solaris
    ;;
    *)
    TARGET=linux
    ;;
esac

echo "Target is $TARGET"

CODE_DIRECTORY=/opt/MARTe
LD_LIBRARY_PATH=.:$CODE_DIRECTORY/BaseLib2/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/MARTe/MARTeSupportLib/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/DataCollectionGAM/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/WebStatisticGAM/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/LinuxTimer/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/GenericTimerDriver/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/CFGUploader/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/CFGUploader/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/SignalHandler/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/MATLABHandler/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/FlotPlot/${TARGET}/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./${TARGET}/




if [ ${TARGET} == "macosx" ]; then
    export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$LD_LIBRARY_PATH
    echo $DYLD_LIBRARY_PATH
else
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
fi

$CODE_DIRECTORY/MARTe/${TARGET}/MARTe.ex $1

#cgdb --args $CODE_DIRECTORY/MARTe/${TARGET}/MARTe.ex $1

