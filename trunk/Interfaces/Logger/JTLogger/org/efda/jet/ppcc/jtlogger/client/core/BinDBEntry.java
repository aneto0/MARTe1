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

import java.net.DatagramPacket;

/**
 *
 * @author andre
 */
public class BinDBEntry extends DefaultDBEntry{
        
    public BinDBEntry(){
        super();
    }
        
    public BinDBEntry(DatagramPacket packet){
        this();
        ipAddress = packet.getAddress();
        ipName = ipAddress.getHostName();
    	ExtractMSG(packet.getData(), packet.getLength());
    }    
    
    /**
        Covert and input byte buffer into an integer. 
    */
    private int toInt(byte[] buf, int start){
        int i = 0; 
        int pos = start;
        i += ((int)(buf[pos++] & 0xFF )) << 24; 
        i += ((int)(buf[pos++] & 0xFF )) << 16;
        i += ((int)(buf[pos++] & 0xFF )) << 8;
        i += ((int)(buf[pos++] & 0xFF )) << 0;
        return i;
    }
     
    /** Parsing of the input stream */
    private void ExtractMSG(byte[] buffer, int bufferLength){

        String msgInformation = new String(buffer, 20, bufferLength-20);
        try{
            errorCode = toInt(buffer, 0);
            time = toInt(buffer, 4);
            taskID = toInt(buffer,8);
            threadID = toInt(buffer,12);            
            objectAddress = toInt(buffer,16);

            int idx   = msgInformation.indexOf(':');
            int idx1  = msgInformation.indexOf(':',idx+1);

            threadName = msgInformation.substring(0,idx);
            className = msgInformation.substring(idx+1,idx1);
            message = msgInformation.substring(idx1+1);
            if(message != null && message.length() > 0)
                message = message.substring(0, message.length() - 1);
        }
        catch (Exception e){
            e.printStackTrace();
        }

        return;
    }
    
}
