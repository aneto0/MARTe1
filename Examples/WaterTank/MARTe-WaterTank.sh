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
#Start-up script for the MARTe WaterTank example
#!/bin/sh

if [ -z "$1" ]; then
	echo "Please specify the location of the configuration file"
	exit
else
	echo "Going to start MARTe with the configuration specified in: " $1
fi

CODE_DIRECTORY=../..
LD_LIBRARY_PATH=.:$CODE_DIRECTORY/BaseLib2/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/MARTe/MARTeSupportLib/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/LinuxTimer/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/GenericTimerDriver/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/StreamingDriver/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/IOGAMs/SynchronizingDriver/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/WebStatisticGAM/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/WaveformGenerator2009/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/DataCollectionGAM/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/NoiseGAM/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/WaterTank/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/PIDGAM/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/GAMs/PlottingGAM/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/CFGUploader/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/SignalHandler/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/MATLABHandler/linux/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CODE_DIRECTORY/Interfaces/HTTP/FlotPlot/linux/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

$CODE_DIRECTORY/MARTe/linux/MARTe.ex $1

#cgdb --args $CODE_DIRECTORY/MARTe/linux/MARTe.ex $1

