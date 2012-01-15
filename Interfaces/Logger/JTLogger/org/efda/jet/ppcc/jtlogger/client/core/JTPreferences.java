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
import java.io.File;
import java.util.prefs.Preferences;

/**
 *
 * @author andre
 */
public class JTPreferences implements IJTPreferences{

    public static String logDirectory = new File(System.getProperty("java.io.tmpdir"), ".jtlogger" + System.currentTimeMillis()).getAbsolutePath();
    public static String allEntriesFile = "all_jt_logger_entries.log";
        
    static{
        try{
            new File(JTPreferences.logDirectory).mkdir();
        }
        catch(Exception e){
            e.printStackTrace();
        }
    }
    
    private static JTPreferences singleton = null;
    public static JTPreferences getJTPreferences(){
        if(singleton == null){
            singleton = new JTPreferences();
        }
        return singleton;
    }
    
    private static Preferences prefs = Preferences.userRoot();
    
    private String PREFERRED_WIDTH_KEY = "PREF_WIDTH_KEY";
    private String PREFERRED_HEIGHT_KEY = "PREF_HEIGHT_KEY";
    private String LOCAL_PORT_KEY = "LOCAL_PORT_KEY";
    private String REMOTE_HOST_KEY = "REMOTE_HOST_KEY";
    private String AUTO_START_KEY = "AUTO_START_KEY";
    private String BACKGROUND_COLOR_KEY = "BACKGROUND_COLOR_KEY";    
    private String MAX_PACKETS_KEY = "MAX_PACKETS_KEY";
    private String UDP_BROADCAST_REFRESH_KEY = "UDP_BROADCAST_REFRESH_KEY";
    private String USE_HISTORY_KEY = "USE_HISTORY_KEY";
    private String N_HISTORY_KEY = "N_HISTORY_KEY";
    private String FONT_SIZE_KEY = "FONT_SIZE_KEY";
    private String NODE_OBSOLETE_SECS_KEY = "NODE_OBSOLETE_SECS_KEY";
    private String DELETE_OBSOLETE_NODES_AUTOMATICALLY_KEY = "DELETE_OBSOLETE_NODES_AUTOMATICALLY_KEY";
        
    private JTPreferences(){        
    }
    
    public Dimension getPreferredDimension(){
        return new Dimension(prefs.getInt(PREFERRED_WIDTH_KEY, 800), prefs.getInt(PREFERRED_HEIGHT_KEY, 600));
    }
    
    public void setPreferredDimension(Dimension dim){
        prefs.putInt(PREFERRED_WIDTH_KEY, (int)dim.getWidth());
        prefs.putInt(PREFERRED_HEIGHT_KEY, (int)dim.getHeight());
    }
    
    public int getLocalPort(){
        return prefs.getInt(LOCAL_PORT_KEY, 32767);
    }
    
    public void setLocalPort(int port){
        prefs.putInt(LOCAL_PORT_KEY, port);
    }
    
    public String getRemoteHost(){
        return prefs.get(REMOTE_HOST_KEY, "127.0.0.1:9099");
    }
    
    public void setRemoteHost(String host){
        prefs.put(REMOTE_HOST_KEY, host);
    }
    
    public boolean isAutoStart(){
        return prefs.getBoolean(AUTO_START_KEY, false);
    }
    
    public void setAutoStart(boolean autoStart){
        prefs.putBoolean(AUTO_START_KEY, autoStart);
    }
    
    public Color getBackgroundColor(){
        int rgb = prefs.getInt(BACKGROUND_COLOR_KEY, 0);        
        return new Color(rgb);
    }
    
    public void setBackgroundColor(Color c){
        prefs.putInt(BACKGROUND_COLOR_KEY, c.getRGB());
    }
    
    public int getMaxPackets(){
        return prefs.getInt(MAX_PACKETS_KEY, 10000);
    }
    
    public void setMaxPackets(int maxPackets){
        prefs.putInt(MAX_PACKETS_KEY, maxPackets);
    }
    
    public int getUDPBroadcastRefreshRate(){
        return prefs.getInt(UDP_BROADCAST_REFRESH_KEY, 500);
    }
    
    public void setUDPBroadcastRefreshRate(int refreshRate){
        prefs.putInt(UDP_BROADCAST_REFRESH_KEY, refreshRate);
    }
    
    public boolean isUseHistory(){
        return prefs.getBoolean(USE_HISTORY_KEY, false);
    }
    
    public void setUseHistory(boolean useHistory){
        prefs.putBoolean(USE_HISTORY_KEY, useHistory);
    }
    
    public int getNumberHistoryItems(){
        return prefs.getInt(N_HISTORY_KEY, 1000);
    }
    
    public void setNumberHistoryItems(int numberItems){
        prefs.putInt(N_HISTORY_KEY, numberItems);
    }
    
    public int getFontSize(){
        return prefs.getInt(FONT_SIZE_KEY, 12);
    }
    
    public void setFontSize(int fontSize){
        prefs.putInt(FONT_SIZE_KEY, fontSize);
    }
    
    public int getNodeObsoleteSeconds(){
        return prefs.getInt(NODE_OBSOLETE_SECS_KEY, 7200);
    }
    
    public void setNodeObsoleteSeconds(int nSeconds){
        prefs.putInt(NODE_OBSOLETE_SECS_KEY, nSeconds);
    }
    
    public boolean deleteObsoleteNodesAutomatically(){
        return prefs.getBoolean(DELETE_OBSOLETE_NODES_AUTOMATICALLY_KEY, true);
    }
    
    public void setDeleteObsoleteNodesAutomatically(boolean delete){
        prefs.putBoolean(DELETE_OBSOLETE_NODES_AUTOMATICALLY_KEY, delete);
    }
}
