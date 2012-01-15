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
BEGIN { FS=" "; Directory="\\"; first=0;path=""}
/.*\.obj.*/{ 
    if (first==0){
        first=1;
    } else {
        spath=sprintf("%s&",path);
        print spath;
    } 
    path=sprintf("+%s\\%s",Directory,$5);
}
/.*Directory.*/{ Directory=$3 }
END { print path }
