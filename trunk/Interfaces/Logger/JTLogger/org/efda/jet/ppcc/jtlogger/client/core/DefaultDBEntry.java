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
import java.util.GregorianCalendar;

/**
 *
 * @author andre
 */
public class DefaultDBEntry implements DBEntry{
    
    private long creationTime;
    
    private long pingTime;
    
    protected long time;
    
    protected int threadID;
    
    protected int taskID;
    
    protected int classID;
    
    protected int objectAddress;
    
    protected int errorCode;
    
    protected static long uniqueIDCounter = 0;
    
    protected long uniqueID = 0;
    
    protected InetAddress ipAddress;
        
    protected String ipName = "";
    
    protected String threadName;
    
    protected String taskName;
    
    protected String className;
    
    protected String objectName;
    
    protected String errorDescription;
    
    protected String errorName;
    
    protected String message;
    
    protected static String LS = System.getProperty("line.separator");
    
    public DefaultDBEntry(){
        uniqueID = uniqueIDCounter++;
        creationTime = System.currentTimeMillis() / 1000;
        ping();
    }
    
    public long getTime() {
        return time;
    }

    public long getKey() {
        return taskID + ipName.hashCode();
    }

    public int getThreadID() {
        return threadID;
    }

    public int getClassID() {
        return classID;
    }

    public int getObjectAddress() {
        return objectAddress;
    }

    public int getErrorCode() {
        return errorCode;
    }
    
    public int getTaskID() {
        return taskID;
    }

    public InetAddress getIP() {
        return ipAddress;
    }

    public String getIPName() {
        return ipName;
    }

    public String getTaskName() {        
        return taskName;
    }

    public String getThreadName() {        
        return threadName;
    }

    public String getClassName() {
         return className;
    }

    public String getObjectName() {        
        return objectName;
    }

    public String getErrorName() {        
        return errorName;
    }

    public String getErrorCodeDescription() {        
        return errorDescription;
    }
    
    public String getMessage(){        
        return message;
    }
    
    public long getUniqueID(){
        return uniqueID;
    }
    
    public long getCreationTime(){
        return creationTime;
    }
    
    public void ping(){
        pingTime = System.currentTimeMillis() / 1000;
    }
    
    public long getLastPingTime(){
        return pingTime;
    }
    
    public String getPrettyCreationTime(){        
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTimeInMillis( creationTime * 1000);
        String date = "" + gc.get(GregorianCalendar.DAY_OF_MONTH);
        date += "/" + gc.get(GregorianCalendar.MONTH);
        date += "/" + gc.get(GregorianCalendar.YEAR);
        date += " @ " + gc.get(GregorianCalendar.HOUR_OF_DAY);
        date += ":" + gc.get(GregorianCalendar.MINUTE);
        date += ":" + gc.get(GregorianCalendar.SECOND);
        return date;
    }
    
    @Override
    public String toString(){
        
        String msg =    "getTime = " + getTime() + LS +
                        "getKey = " + getKey() + LS +
                        "getThreadID = " + getThreadID() + LS +
                        "getTaskID = " + getTaskID() + LS +
                        "getClassID = " + getClassID() + LS +
                        "getObjectAddress = " + getObjectAddress() + LS +
                        "getErrorCode = " + getErrorCode() + LS +
                        "getIPName = " + getIPName() + LS +
                        "getTaskName = " + getThreadName() + LS +
                        "getClassName = " + getClassName() + LS +
                        "getErrorName = " + getErrorName() + LS +
                        "getErrorCodeDescription = " + getErrorCodeDescription() + LS +
                        "getObjectName = " + getObjectName() + LS +
                        "getMessage = " + getMessage();
                    
        return msg;
    }
}
