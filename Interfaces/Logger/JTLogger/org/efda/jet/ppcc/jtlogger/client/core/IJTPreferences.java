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
package org.efda.jet.ppcc.jtlogger.client.core;

import java.awt.Color;
import java.awt.Dimension;

/**
 *
 * @author andre
 */
public interface IJTPreferences {
    public Dimension getPreferredDimension();
    public int getLocalPort();
    public String getRemoteHost();
    public boolean isAutoStart();
    public Color getBackgroundColor();
    public int getMaxPackets();
    public int getUDPBroadcastRefreshRate();
    public int getFontSize();
    public int getNodeObsoleteSeconds();
    public boolean deleteObsoleteNodesAutomatically();
}
