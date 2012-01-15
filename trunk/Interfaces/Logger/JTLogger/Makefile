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

CLASSPATH = .

JFLAGS = -classpath $(CLASSPATH) -target 1.5
JCFLAGS =
JRFLAGS =
BUILD_DIR = build
JAVAC = javac $(JFLAGS) -d $(BUILD_DIR)

.SUFFIXES : .java .class

all : jar

compile :
	$(shell find . -name \*.java -print > file.compile.list)
	$(JAVAC) @file.compile.list

jar : compile
	rm -f $(BUILD_DIR)/JTLogger.jar
	jar cvfm $(BUILD_DIR)/JTLogger.jar Manifest.mf -C $(BUILD_DIR) org
	jar uvf $(BUILD_DIR)/JTLogger.jar org/efda/jet/ppcc/jtlogger/client/resources/
	cp $(BUILD_DIR)/JTLogger.jar JTLogger.jar

doc :
	mkdir -p doc
	javadoc -d doc *.java

clean :
	rm -rf doc
	rm -rf $(BUILD_DIR)/* 
	rm JTLogger.jar
