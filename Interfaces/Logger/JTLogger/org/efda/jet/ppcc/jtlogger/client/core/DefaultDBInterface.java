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

import java.util.Vector;
import javax.swing.event.EventListenerList;

/**
 *
 * @author andre
 */
public class DefaultDBInterface implements DBInterface, PreferenceChangeListener{

    private int maxEntries = 10000;
    
    private Vector<DBEntry> entries = new Vector<DBEntry>();    
        
    public Vector<DBEntry> filter(long key, KeyType[] type, int[] value){
        if(key == 0)
            return entries;       
        
        Vector<DBEntry> toRet = new Vector<DBEntry>(entries);
        if(type.length != 0){
            for(int i=0; i<type.length; i++){            
                toRet = search(toRet, key, type[i], value[i], false);
            }
        }
        else{
            toRet = search(toRet, key, KeyType.NONE, -1, false);
        }
        
        if(toRet.size() > 500)
            toRet.setSize(500);
        return toRet;
    }
    
    private Vector<DBEntry> search(Vector<DBEntry> entries, long key, KeyType type, int value, boolean fast){
        Vector<DBEntry> toRet = new Vector<DBEntry>();
        DBEntry entry = null;
        boolean found;
        int toCompare = 0;
        for(int i=0; i<entries.size(); i++){
            found = false;
            if(entries.get(i).getKey() == key){
                switch(type){
                    case THREADS:
                        toCompare = entries.get(i).getThreadID();
                        break;
                    case ERROR:
                        toCompare = entries.get(i).getErrorCode();
                        break;
                    case OBJECTS:
                        toCompare = entries.get(i).getObjectAddress();
                        break;
                    case CLASSES:
                        toCompare = entries.get(i).getClassID();
                        break;
                    case NONE:
                        toCompare = value;
                        break;
                }
                if(value == toCompare){
                    entry = entries.get(i);
                    found = true;
                }                                
            }
            if(found && entry != null){
                toRet.add(entry);
                if(fast)
                    return toRet;
            }
            
            entry = null;
        }        
        
        return toRet;
    }

    public void addDBEntry(DBEntry entry) {
        entries.add(entry);    
    }

    public void removeDBEntry(DBEntry entry) {
        entries.remove(entry);
    }

    public void newEntriesAvailables(Vector<DBEntry> list) {
        entries.addAll(list);
        checkMaxSize();
    }
    
    public void shutdown(){
        
    }
    
    private void checkMaxSize(){
        Vector<DBEntry> removedEntries = new Vector<DBEntry>();
        while(entries.size() > maxEntries){
            removedEntries.add(entries.remove(0));
        }
        
        if(removedEntries.size() > 0){
            fireDBEntriesRemoved(removedEntries);
        }
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
    
    public void preferencesChanged() {
        maxEntries = JTPreferences.getJTPreferences().getMaxPackets();
    }
    
    public void addDBEntryRemoveListener(DBEntriesRemovedListener listener){
        listenersList.add(DBEntriesRemovedListener.class, listener);
    }
    
    public void removeDBEntryRemoveListener(DBEntriesRemovedListener listener){
        listenersList.remove(DBEntriesRemovedListener.class, listener);
    }    
}
