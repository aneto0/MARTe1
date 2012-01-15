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
import java.net.InetAddress;

/**
 *
 * @author andre
 */
public class TextDBEntry extends BinDBEntry{

    public TextDBEntry(){
        super();
    }
    
    public TextDBEntry(String textEntry){
        this();
        //System.out.println("textEntry=" + textEntry);
        String[] comps = textEntry.split("\\|");
        for(int i=0; i<comps.length; i++){            
            String[] item = comps[i].split("=", 2);
            if(item.length == 2){
                if(item[0].equals("TM")){
                    time = readLongFromHexaString(item[1]);
                }
                else if(item[0].equals("C")){
                    className = item[1];
                }
                else if(item[0].equals("N")){
                    threadName = item[1];
                }
                else if(item[0].equals("I")){
                    try{
                        ipAddress = InetAddress.getByName(item[1]);
                        ipName = ipAddress.getHostName();
                    }
                    catch(Exception e){
                        e.printStackTrace();
                    }
                }
                else if(item[0].equals("O")){
                    objectAddress = readIntFromHexaString(item[1]);
                }
                else if(item[0].equals("T")){
                    threadID = readIntFromHexaString(item[1]);                    
                }
                else if(item[0].equals("P")){
                    taskID = readIntFromHexaString(item[1]);                    
                }
                else if(item[0].equals("E")){
                    errorCode = readIntFromHexaString(item[1]);                    
                }
                else if(item[0].equals("D")){
                    message = item[1];
                }
            }
        }        
    }
    
    public TextDBEntry(DatagramPacket packet){
        this(new String(packet.getData(), 0, packet.getLength()));
    }
    
    private int readIntFromHexaString(String hexa){
        return (int)readLongFromHexaString(hexa);
    }
    
    private long readLongFromHexaString(String hexa){
        try{
            return Long.parseLong(hexa, 16);
        }
        catch(NumberFormatException nfe){
            nfe.printStackTrace();
        }
        return 0;
    }
}
