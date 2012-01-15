/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/

/**
 * @file
 * @brief Colormaps support
 */
#if !defined (COLORMAP_INTERFACE_H)
#define COLORMAP_INTERFACE_H

#include "System.h"
#include "ErrorManagement.h"

typedef struct {
    int32  red;
    int32  green;
    int32  blue;
} sColormap;

class ColormapInterface {

protected:

  // Number of colors in colormap
  int32             nColors;

  // Array of sColormap structures
  sColormap        *cMap;

  // Start value of the range to be mapped
  float             startVal;

  // End value of the range to be mapped
  float             endVal;

protected:

  // Build color table
  virtual bool BuildJetColorMap() = 0;
  
public:

  /// Default constructor
  ColormapInterface() {
      cMap = NULL;
      nColors = 0;
      startVal = 0.0;
      endVal   = 0.0;
  }

  /// Alternative constructor
  ColormapInterface(int32 numberOfColors) {
      startVal = 0.0;
      endVal   = 0.0;
      SetSize(numberOfColors);
  }

  /// Destructor
  ~ColormapInterface() {
      if(cMap != NULL) {
          free((void*&)cMap);
	  cMap = NULL;
      }
  }

  /// Set number of colors in colormap (color table)
  bool SetSize(int32 numberOfColors) {
      if(cMap != NULL) {
          free((void*&)cMap);
      	  cMap = NULL;
      }
      if((cMap = (sColormap *)malloc(numberOfColors*sizeof(sColormap))) == NULL) {
	  CStaticAssertErrorCondition(InitialisationError,"EventGAM::Colormap: unable to allocate memory for colormap table");
	  nColors = 0;
	  startVal = 0.0;
	  endVal   = 0.0;
	  return False;
      } else {
	  nColors = numberOfColors;
      }
      return BuildJetColorMap();
  }

  void SetStartValue(float startValue) {
      startVal = startValue;
  }

  void SetEndValue(float endValue) {
      endVal = endValue;
  }

  sColormap *CMap() {
      return cMap;
  }

  int32 NColors() {
      return nColors;
  }

  float StartValue() {
      return startVal;
  }

  float EndValue() {
      return endVal;
  }

  bool GetColorCode(float value, sColormap *color) {
      if((endVal <= startVal) || startVal == 0) {
	  CStaticAssertErrorCondition(InitialisationError, "EventGAM::Colormap: GetColorCode() endVal <= startVal or startVal == 0");
	  return False;
      }
      if(color == NULL) {
	  CStaticAssertErrorCondition(InitialisationError, "EventGAM::Colormap: GetColorCode() dest point is null");
	  return False;
      }
      int32 idx;
      if(value <= startVal) {
	  idx = 0;
      } else if(value >= endVal) {
	  idx = nColors-1;
      } else {
	  float b = (float)nColors/(1.0-endVal/startVal);
	  idx = (int32)floor((-b/startVal)*value + b);
      }
      memcpy(color, &cMap[idx], sizeof(sColormap));
      return True;
  }
};
#endif

