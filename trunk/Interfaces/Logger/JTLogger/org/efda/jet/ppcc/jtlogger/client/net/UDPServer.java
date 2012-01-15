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
package org.efda.jet.ppcc.jtlogger.client.net;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.util.Vector;
import javax.swing.event.EventListenerList;
import org.efda.jet.ppcc.jtlogger.client.core.BinDBEntry;
import org.efda.jet.ppcc.jtlogger.client.core.DBEntry;
import org.efda.jet.ppcc.jtlogger.client.core.IJTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.InternalLogErrorLevel;
import org.efda.jet.ppcc.jtlogger.client.core.InternalLogListener;
import org.efda.jet.ppcc.jtlogger.client.core.JTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.LoggerEntryListener;
import org.efda.jet.ppcc.jtlogger.client.core.PreferenceChangeListener;
import org.efda.jet.ppcc.jtlogger.client.core.TextDBEntry;
import org.efda.jet.ppcc.jtlogger.client.core.UDPServerStatusListener;

/**
 *
 * @author andre
 */
public class UDPServer implements PreferenceChangeListener{
    
    private DatagramSocket socket = null;
    private boolean keepAlive = true;
    private byte[] buffer = new byte[512];
    private Vector<DBEntry> messageBuffer = null;    
    private long milliSecMessageSend = 1000;
    private UDPListener udpListener = null;
    private UDPMessageBroadcaster udpMessageBroadcaster = null;
    
    //Defines the number of DBEntries to be fired each time
    private int maxCopySize = 1000;
        
    public UDPServer(long milliSecMessageSend){
        this.milliSecMessageSend = milliSecMessageSend;
        messageBuffer = new Vector<DBEntry>();        
    }
    
    public boolean start(int port){
        keepAlive = true;
        fireServerStatusChanged(UDPServerStatusListener.ServerState.STARTING);
        try{
            socket = new DatagramSocket(port);
        }
        catch(IOException ioe){
            fireServerStatusChanged(UDPServerStatusListener.ServerState.ERROR, ioe.getMessage());
            return false;
        }
        
        udpMessageBroadcaster = new UDPMessageBroadcaster();
        udpMessageBroadcaster.start();
        udpListener = new UDPListener();
        udpListener.start();

	new GarbageCleaner().start();

        fireServerStatusChanged(UDPServerStatusListener.ServerState.STARTED);
        return true;
    }        
    
    private class GarbageCleaner extends Thread{
	public void run(){
		while(keepAlive){
	                Runtime.getRuntime().gc();
			try{
				sleep(20000);
			}
			catch(InterruptedException ignored){}
		}
	}
    }


    private class UDPMessageBroadcaster extends Thread{
        
        private long start = 0;
        private long sleepTime = 0;
        private boolean wasStressed = false;
        
        @Override
        public void run(){
            while(messageBuffer.size() != 0 || keepAlive){
                try{
                    start = System.currentTimeMillis();
                    if(messageBuffer.size() != 0){
                        
                        Vector<DBEntry> temp = new Vector<DBEntry>(messageBuffer);
                        if(messageBuffer.size() > maxCopySize){
                            temp.setSize(maxCopySize);                            
                        }
                        fireNewEntriesAvailable(temp);
                        messageBuffer.removeAll(temp);                        
                    }
                    sleepTime = milliSecMessageSend - (System.currentTimeMillis() - start);                    
                    if(sleepTime > 0){
                        if(wasStressed){
                            fireNewInternalLogMessage(InternalLogErrorLevel.INFO, "UDP Broadcaster is now ok! Sleeping for: " + sleepTime + " ms");
                            wasStressed = false;
                        }
                        sleep(sleepTime);                        
                    }
                    else{
                        fireNewInternalLogMessage(InternalLogErrorLevel.WARNING, "UDP Broadcaster is under stress!");
                        wasStressed = true;
                        sleep(50);                        
                    }
                }
                catch(InterruptedException ie){
                    ie.printStackTrace();
                }
            }
            
            fireServerStatusChanged(UDPServerStatusListener.ServerState.STOPPED);
        }        
    }
    
    private class UDPListener extends Thread{

        private DatagramPacket packet = null;

        public UDPListener(){
            setPriority(Thread.MAX_PRIORITY);
        }
        
        @Override
        public void run(){
            while(keepAlive){
                try{                    
                    // receive request
                    packet = new DatagramPacket(buffer, buffer.length);
                    socket.receive(packet);
                    if(packet.getData().length > 0){
                        if(packet.getData()[0] == '|'){
                            messageBuffer.add(new TextDBEntry(packet));
                        }
                        else{
                            messageBuffer.add(new BinDBEntry(packet));
                        }
                    }
                }
                catch(IOException ioe){
                    if(!(ioe instanceof SocketException)){
                        ioe.printStackTrace();
                        fireServerStatusChanged(UDPServerStatusListener.ServerState.ERROR, ioe.getMessage());
                    }                    
                }                
            }
            
            shutdown();
        }
        
        public void shutdown(){
            if(socket != null && !socket.isClosed())
                socket.close();
        }
    }
    
    public void shutdown(){
        if(udpListener == null)
            return;
        udpListener.shutdown();
        fireServerStatusChanged(UDPServerStatusListener.ServerState.STOPPING);
        keepAlive = false;
    }
    
    public void preferencesChanged() {
        milliSecMessageSend = JTPreferences.getJTPreferences().getUDPBroadcastRefreshRate();
    }
        
    private EventListenerList listenersList = new EventListenerList();
    
    private void fireNewEntriesAvailable(Vector<DBEntry> toSend){
        if (listenersList == null) return;
        Object[] listeners = listenersList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==LoggerEntryListener.class)
            {
                ((LoggerEntryListener)listeners[i+1]).newEntriesAvailables(toSend);
            }
        }        
    }
    
    private void fireNewInternalLogMessage(InternalLogErrorLevel level, String message){
        long time = System.currentTimeMillis();
        if (listenersList == null) return;
        Object[] listeners = listenersList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==InternalLogListener.class){                
                ((InternalLogListener)listeners[i+1]).newLogMessageAvailable(level, time, message);
            }
        }        
    }
    
    private void fireServerStatusChanged(UDPServerStatusListener.ServerState newState, String extraInfo){
        if (listenersList == null) return;
        Object[] listeners = listenersList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==UDPServerStatusListener.class){                
                ((UDPServerStatusListener)listeners[i+1]).serverStatusChanged(newState, extraInfo);
            }
        }        
    }
    
    private void fireServerStatusChanged(UDPServerStatusListener.ServerState newState){
        fireServerStatusChanged(newState, null);
    }
    
    public void addLoggerEntryListener(LoggerEntryListener listener){
        listenersList.add(LoggerEntryListener.class, listener);
    }
    
    public void removeLoggerEntryListener(LoggerEntryListener listener){
        listenersList.remove(LoggerEntryListener.class, listener);
    }            
    
    public void addInternalLogListener(InternalLogListener listener){
        listenersList.add(InternalLogListener.class, listener);
    }
    
    public void removeInternalLogListener(InternalLogListener listener){
        listenersList.remove(InternalLogListener.class, listener);
    }
    
    public void addUDPServerStatusListener(UDPServerStatusListener listener){
        listenersList.add(UDPServerStatusListener.class, listener);
    }
    
    public void removeUDPServerStatusListener(UDPServerStatusListener listener){
        listenersList.remove(UDPServerStatusListener.class, listener);
    }
        
}
