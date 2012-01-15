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
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Enumeration;
import java.util.GregorianCalendar;
import java.util.Hashtable;
import java.util.Vector;
import javax.swing.event.EventListenerList;

/**
 *
 * @author andre
 */
public class FileDBInterface implements DBInterface, PreferenceChangeListener{

    private Hashtable<String, RandomAccessFile> fileTable = new Hashtable<String, RandomAccessFile>();
    private Hashtable<String, Long> fileTimes = new Hashtable<String, Long>();
    private String LS = System.getProperty("line.separator");
    private RandomAccessFile globalFile = null;
    private int maxGlobalFileLength = 1000000;
    private int fontSize = 12;
    
    public FileDBInterface(){
        try{
            globalFile = new RandomAccessFile(new File(JTPreferences.logDirectory, JTPreferences.allEntriesFile), "rw");
        }
        catch(Exception e){
            e.printStackTrace();
        }
        
        new FileCloser().start();
    }
    
    public void addDBEntry(DBEntry entry) {
        RandomAccessFile ras = checkFileEntry(entry);
        try{
            ras.writeBytes(getFormattedMessage(entry) + LS);
            globalFile.writeBytes(getFormattedMessage(entry) + LS);
            if(globalFile.length() > maxGlobalFileLength){
                globalFile.setLength(maxGlobalFileLength / 2);
            }
        }
        catch(IOException ioe){
            ioe.printStackTrace();
        }
    }

    public void removeDBEntry(DBEntry entry) {        
    }

    private EventListenerList listenersList = new EventListenerList();
    
    private void fireDBEntriesRemoved(Vector<DBEntry> entries){
        if (listenersList == null) return;
        Object[] listeners = listenersList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i]==DBEntriesRemovedListener.class)
            {
                ((DBEntriesRemovedListener)listeners[i+1]).dbEntriesRemoved(entries);
            }
        }        
    }
    
    public void addDBEntryRemoveListener(DBEntriesRemovedListener listener){
        listenersList.add(DBEntriesRemovedListener.class, listener);
    }
    
    public void removeDBEntryRemoveListener(DBEntriesRemovedListener listener){
        listenersList.remove(DBEntriesRemovedListener.class, listener);
    }    

    public Vector<DBEntry> filter(long key, KeyType[] type, int[] value) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public void newEntriesAvailables(Vector<DBEntry> list) {
        for(int i=0; i<list.size(); i++){
            addDBEntry(list.get(i));
        }            
    }

    public void preferencesChanged() {
        maxGlobalFileLength = JTPreferences.getJTPreferences().getMaxPackets() * 500;
        fontSize = JTPreferences.getJTPreferences().getFontSize();
    }
    
    public void shutdown(){
        try{
            globalFile.close();            
            Enumeration<String> keys = fileTable.keys();            
            while(keys.hasMoreElements()){
                fileTable.remove(keys.nextElement()).close();
            }
        }
        catch(IOException ioe){
            ioe.printStackTrace();   
        }        
    }

    private RandomAccessFile checkFileEntry(DBEntry entry){
        String id = entry.getKey() + "";
        if(fileTable.get(id) != null){
            return fileTable.get(id);
        }
        
        RandomAccessFile ras = null;
        try{
            ras = new RandomAccessFile(new File(JTPreferences.logDirectory, "A" + entry.getKey() + ".log"), "rw");
            ras.writeBytes("<html>" + LS);            
            fileTable.put(id, ras);
            fileTimes.put(id, System.currentTimeMillis());
        }
        catch(Exception e){
            e.printStackTrace();
            return null;
        }        
        return ras;
    }
    
    private String getFormattedMessage(DBEntry entry){
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTimeInMillis(entry.getTime() * 1000);
        String date = "" + gc.get(GregorianCalendar.HOUR_OF_DAY);
        date += ":" + gc.get(GregorianCalendar.MINUTE);
        date += ":" + gc.get(GregorianCalendar.SECOND);
        
        String source = entry.getIPName();        
        String error = ErrorStringTable.getError(entry.getErrorCode());
        if(error == null){
            error = "ERROR " + entry.getErrorCode();
        }
        
        String tidHex = String.format("%x", entry.getThreadID());
        String cidHex = String.format("%x", entry.getClassID());
        String objectPointerHex = String.format("%x", entry.getObjectAddress());        
        
        String tName = entry.getThreadName();
        String cName = entry.getClassName();
        
        String composedMSG = "[" + date + "]:" + source + ":";
        composedMSG += error;
        
        if(tName == null){
            composedMSG += ":tid=0x" + tidHex;
        }
        else{
            composedMSG += ":tid=0x" + tidHex + " (" + tName + ")";
        }
        
        
        composedMSG += ":cid=0x" + cidHex;                
        if(entry.getObjectAddress() != 0){
            if(cName != null){
                composedMSG += ":obj=(" + cName + " *)0x" + objectPointerHex;
            }
            else{
                composedMSG += ":obj=0x" + objectPointerHex;
            }
        }
        else{
            composedMSG += ":obj=GLOBAL";
        }
        
        String msg = entry.getMessage();
	if(msg != null){
	        msg = msg.replaceAll("&", "&amp;");
	        msg = msg.replaceAll("<", "&lt;");
	        msg = msg.replaceAll(">", "&gt;");        
	        msg = msg.replaceAll("\"", "&quot;");          
	}else{
		msg = "";
	}
        composedMSG += ":" + msg;
        if(composedMSG.endsWith(LS)){
            composedMSG = composedMSG.substring(0, composedMSG.length() - 1);
        }
        return "<p style=\"font-size:" + fontSize + ";color:" + colorToHTML(ErrorColorTable.getColor(entry.getErrorCode())) + "\">" + composedMSG + "</p>";
    }
    
    static String colorToHTML(Color color) {
        String hex = "00000" + Integer.toHexString(color.getRGB());
        return "#" + hex.substring(hex.length() - 6);
    }
    
    private class FileCloser extends Thread{
        public void run(){
            try{
                sleep(3600000);
            }
            catch(Exception e){
                e.printStackTrace();
            }
            Enumeration<String> keys = fileTimes.keys();
            String key;
            long time = 0;
            while(keys.hasMoreElements()){
                key = keys.nextElement();
                time = fileTimes.get(key).longValue();
                if((System.currentTimeMillis() - time) > 3600000){
                    try{
                        fileTable.remove(key).close();
                        fileTimes.remove(key);
                    }catch(Exception e){
                        e.printStackTrace();
                    }
                }
            }            
        }
    }
}
