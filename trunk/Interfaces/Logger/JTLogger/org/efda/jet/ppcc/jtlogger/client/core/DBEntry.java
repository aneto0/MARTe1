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

import java.net.InetAddress;

/**
 *
 * @author andre
 */
public interface DBEntry {        
    
    public long getTime();
    
    /** Selection keys*/
    //proc + machine
    public long getKey();
    
    public int getThreadID();
    
    public int getTaskID();
    
    public int getClassID();
    
    public int getObjectAddress();
    
    public int getErrorCode();
            
    //For internal use only!
    public long getUniqueID();
    
    /** Visualization*/    
    public InetAddress getIP();
    
    public String getIPName();
        
    public String getTaskName();
    
    public String getThreadName();
    
    public String getClassName();
    
    public String getObjectName();
    
    public String getErrorName();
    
    public String getErrorCodeDescription();
    
    public String getMessage();
    
    public long getCreationTime();
    
    public String getPrettyCreationTime();
    
    public long getLastPingTime();
    
    public void ping();
}
