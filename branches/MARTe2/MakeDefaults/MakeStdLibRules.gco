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

ctags : %.cpp %.c %.h
	ctags -R

$(BUILD_DIR)/%.o : %.cpp
	$(COMPILER) -c $(OPTIM) $(CFLAGS) $(CPPFLAGS) $(CFLAGSPEC) $(DEBUG)  $*.cpp -o $(BUILD_DIR)/$*.o
#	$(COMPILER) -S $(OPTIM) $(CFLAGS) $(CFLAGSPEC) $(DEBUG)  $*.cpp -o $(BUILD_DIR)/$*.s

$(BUILD_DIR)/%.o : %.c
	$(CCOMPILER) -c $(CFLAGS) $(CFLAGSPEC) $(DEBUG) $(OPTIM)  $*.c -o $(BUILD_DIR)/$*.o
#	$(COMPILER) -S $(CFLAGS) $(CFLAGSPEC) $(DEBUG) $(OPTIM)  $*.c -o $(BUILD_DIR)/$*.s

$(BUILD_DIR)/%.a : $(OBJS)
	touch $@
	rm $(BUILD_DIR)/$*.a
	ld -r $(OBJS) -o $@

$(BUILD_DIR)/%.so : $(BUILD_DIR)/%.o $(OBJS)
#	$(COMPILER) -shared -fPIC $(OBJS) $(LIBRARIES) $(BUILD_DIR)/$*.o -o $@
	$(COMPILER) $(LFLAGS) -shared -fPIC $(OBJS) $(BUILD_DIR)/$*.o $(LIBRARIES) -o $@
	touch $(BUILD_DIR)/lib$*.so
	rm  $(BUILD_DIR)/lib$*.so
	ln -fns $*.so $(BUILD_DIR)/lib$*.so

%.ifo :  %.h
	$(COMPILER) -E -I$(CINTSYSDIR)/src  -I$(CINTSYSDIR) $(CFLAGS) $(DEBUG) $(OPTIM)  $*.h > temp/temp.pp
	cint  -i temp/temp.pp $(MAKEDEFAULTDIR)/MakeIfo.cxx $* > $*.ifo

%.sinfo.cpp :  %.h
	$(COMPILER) -D_CINT -E -I$(CINTSYSDIR)/src  -I$(CINTSYSDIR) $(CFLAGS) $(DEBUG) $(OPTIM)  $*.h > temp/temp.pp
	cint   -i  $(MAKEDEFAULTDIR)/MakeIfo.cxx $* > $*.sinfo.cpp

$(BUILD_DIR)/%.exe : $(BUILD_DIR)/%.o 
	touch $(BUILD_DIR)/$*.exe
	echo cannot build executable $(BUILD_DIR)/$*.exe use $(BUILD_DIR)/$*.ex

$(BUILD_DIR)/%.ex : $(BUILD_DIR)/%.o $(OBJS)
	$(COMPILER) $(LFLAGS) $(BUILD_DIR)/$*.o $(OBJS) $(LIBRARIES)  -o $(BUILD_DIR)/$*.ex

depends.gco: dependsRaw.gco
	@echo "/\.o:/s/^/$(subst /,\/,$(BUILD_DIR))\//" > CreateLinuxDepends.sed
	sed -f CreateLinuxDepends.sed dependsRaw.gco >depends.gco
	rm -f CreateLinuxDepends.sed 

dependsRaw.gco: 
	$(COMPILER) $(CFLAGS) $(CFLAGSPEC) $(DEBUG) $(OPTIM) -MM -I. *.c* > dependsRaw.gco
	@mkdir -p temp
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.gam : $(BUILD_DIR)/%.o $(OBJS)
	$(COMPILER) $(LFLAGS) -shared -fPIC $(OBJS) $(LIBRARIES) $(BUILD_DIR)/$*.o -o $@

$(BUILD_DIR)/%.drv : $(BUILD_DIR)/%.o $(OBJS)
	$(COMPILER) $(LFLAGS) -shared -fPIC $(OBJS) $(LIBRARIES) $(BUILD_DIR)/$*.o -o $@

%.spb : 
	$(MAKE) -C $* -f Makefile.$(TARGET)


%.spc : 
	$(MAKE) -C $* -f Makefile.$(TARGET) clean


clean:  $(SUBPROJC)
	@rm -f depends*
	@rm -f $(BUILD_DIR)/*$(OBJEXT)
	@rm -f $(BUILD_DIR)/*$(DLLEXT)
	@rm -f $(BUILD_DIR)/*$(EXEEXT)
	@rm -f $(BUILD_DIR)/*$(LIBEXT)
	@rm -f $(BUILD_DIR)/*$(GAMEXT)
	@rm -f $(BUILD_DIR)/*$(DRVEXT)
	@rm -f $(BUILD_DIR)/*$(ASMEXT)
	@rm -f $(BUILD_DIR)/*$(GCNOEXT)
	@rm -f $(BUILD_DIR)/*$(GCDAEXT)

