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
package org.efda.jet.ppcc.jtlogger.client;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Vector;
import javax.swing.AbstractAction;
import javax.swing.ImageIcon;
import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.RootPaneContainer;
import javax.swing.SwingUtilities;
import javax.swing.ToolTipManager;
import javax.swing.border.LineBorder;
import javax.swing.event.EventListenerList;
import org.efda.jet.ppcc.jtlogger.client.core.DBEntry;
import org.efda.jet.ppcc.jtlogger.client.core.DBInterface;
import org.efda.jet.ppcc.jtlogger.client.core.ErrorColorTable;
import org.efda.jet.ppcc.jtlogger.client.core.FileDBInterface;
import org.efda.jet.ppcc.jtlogger.client.core.IJTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.InternalLogErrorLevel;
import org.efda.jet.ppcc.jtlogger.client.core.InternalLogListener;
import org.efda.jet.ppcc.jtlogger.client.core.JTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.LoggerEntryListener;
import org.efda.jet.ppcc.jtlogger.client.core.PreferenceChangeListener;
import org.efda.jet.ppcc.jtlogger.client.core.UDPServerStatusListener;
import org.efda.jet.ppcc.jtlogger.client.net.UDPServer;
import org.efda.jet.ppcc.jtlogger.client.ui.ConnectDialog;
import org.efda.jet.ppcc.jtlogger.client.ui.InternalLogDialog;
import org.efda.jet.ppcc.jtlogger.client.ui.InternalLogLabel;
import org.efda.jet.ppcc.jtlogger.client.ui.LoggerTextPane;
import org.efda.jet.ppcc.jtlogger.client.ui.LoggerTreeCellRenderer;
import org.efda.jet.ppcc.jtlogger.client.ui.LoggerTree;
import org.efda.jet.ppcc.jtlogger.client.ui.MemoryButton;
import org.efda.jet.ppcc.jtlogger.client.ui.PreferencesDialog;

/**
 *
 * @author andre
 */
public class JTLogger implements LoggerEntryListener, UDPServerStatusListener, PreferenceChangeListener{    
    
    private DBInterface ddbInterface = null;
    
    //The main container
    private Container mainContainer = null;
    
    //The logger tree
    private LoggerTree loggerTree = null;
    
    //The text pane
    private LoggerTextPane loggerText = null;
    private JScrollPane loggerTextScrollPane = null;
    
    //The UDP server
    private UDPServer server = null;        
        
    //The small logo
    private ImageIcon LOGO_16 = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/logo16.png"));
    
    //The connect icon
    private ImageIcon CONNECT_16 = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/connect.png"));
    
    //The disconnect icon
    private ImageIcon DISCONNECT_16 = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/disconnect.png"));
    
    //The internal logger icon
    private ImageIcon ILOGGER_16 = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/internallogger.png"));
    
    //The preferences icon
    private ImageIcon PREFS_16 = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/preferences.png"));        
  
    //The clear log icon
    private ImageIcon CLEAR_16 = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/logger.png"));
    
    //The user preferences
    private JTPreferences jtPrefs = JTPreferences.getJTPreferences();
    
    //The connect dialog
    private ConnectDialog connectDialog = null;
    
    //The message label
    private InternalLogLabel internalLogMessageLabel = null;
    
    //Label which keeps track of the number of packets received
    private JTextField  packetCounterTextField = null;
    private JLabel      packetCounterLabel = null;

    //Text field to perform searching
    private JTextField  searchField = null;

    //The number of packets received
    private long packetCounter = 0;
    
    //The connect button
    private JButton connectButton = null;
    
    //The disconnect button
    private JButton disconnectButton = null;
    
    //The clear log button
    private JButton clearLogButton = null;
    
    //The internal log dialog
    private InternalLogDialog internalLogDialog = null;
    
    //Start automatically?
    private boolean autoStart = false;
    
    //The preferences dialog
    private PreferencesDialog prefsDialog = null;
    
    //UDP Refresher object sync
    private Object UDPConnectorSync = new Object();
    private boolean UDPConnectorRunning = false;
    
    private Frame dialogContainer;
    
    public JTLogger(Container mainContainer){
        clearTempFiles();
        this.mainContainer = mainContainer;
        dialogContainer = (Frame)SwingUtilities.getAncestorOfClass(Frame.class, mainContainer);
        //ddbInterface = new DefaultDBInterface();
        ddbInterface = new FileDBInterface();
        
        initComponents();
        server = new UDPServer(500);        
        server.addLoggerEntryListener(loggerTree);
        server.addLoggerEntryListener(ddbInterface);
        server.addLoggerEntryListener(this);
        server.addUDPServerStatusListener(this);
        server.addInternalLogListener(internalLogMessageLabel);
        server.addInternalLogListener(internalLogDialog);        
        loggerTree.addLoggerFilterListener(loggerText);        
        ddbInterface.addDBEntryRemoveListener(loggerTree);
        
        prefsDialog.addPreferenceChangeListener(this);
        prefsDialog.addPreferenceChangeListener(loggerText);
        prefsDialog.addPreferenceChangeListener(internalLogMessageLabel);
        prefsDialog.addPreferenceChangeListener(internalLogDialog);
        prefsDialog.addPreferenceChangeListener(ddbInterface);
        prefsDialog.addPreferenceChangeListener(server);
        prefsDialog.requestPreferencesUpdate();
        
        autoStart = jtPrefs.isAutoStart();
        
        if(autoStart)
            startConnections();
    }
    
    private void initComponents(){
        if(mainContainer instanceof JFrame){
            ((JFrame)mainContainer).setIconImage(LOGO_16.getImage());
        }
        loggerTree = new LoggerTree(ddbInterface);
        loggerTree.setCellRenderer(new LoggerTreeCellRenderer());        
        JToolBar toolbar = new JToolBar();
        JScrollPane treePane = new JScrollPane(loggerTree);
        treePane.setPreferredSize(new Dimension(240, 400));
                
        loggerText = new LoggerTextPane(ddbInterface);
        //Workaround to avoid wraping on the text pane
        JPanel workaroundPanel = new JPanel(new BorderLayout());
        workaroundPanel.add(loggerText);
        loggerTextScrollPane = new JScrollPane(workaroundPanel);
        loggerText.setContainerPane(loggerTextScrollPane);
        loggerTextScrollPane.getViewport().setBackground(Color.BLACK);        
        MemoryButton mb = new MemoryButton(5000);
        
        JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
        
        splitPane.setLeftComponent(treePane);
        splitPane.setRightComponent(loggerTextScrollPane);
        splitPane.setDividerLocation(0.4);
        splitPane.setDividerSize(2);
        
        connectButton = new JButton(CONNECT_16);
        connectButton.setToolTipText("Connect UDP server to remote host");
        connectButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                handleConnectionRequest(e);
            }
        });
        
        disconnectButton = new JButton(DISCONNECT_16);
        disconnectButton.setToolTipText("Disconnect UDP server");
        disconnectButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                handledisconnectRequest(e);
            }
        });
        disconnectButton.setEnabled(false);
        
        clearLogButton = new JButton(CLEAR_16);
        clearLogButton.setToolTipText("Clear log window");
        clearLogButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                handleClearLog(e);
            }
        });
        clearLogButton.setEnabled(true);
        
        JButton iloggerButton = new JButton(ILOGGER_16);
        iloggerButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                handledInternalLoggerButtonRequest(e);
            }
        });
        
        JButton prefsDialogButton = new JButton(PREFS_16);
        prefsDialogButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                handledPrefsDialogButtonRequest(e);
            }
        });
                
        prefsDialog = new PreferencesDialog(dialogContainer, jtPrefs);
                
        toolbar.add(connectButton);
        toolbar.add(disconnectButton);
        toolbar.add(clearLogButton);
        toolbar.add(iloggerButton);
        toolbar.add(prefsDialogButton);
        toolbar.add(mb);

        connectDialog = new ConnectDialog(dialogContainer, jtPrefs.getLocalPort(), jtPrefs.getRemoteHost(), jtPrefs.isUseHistory(), jtPrefs.getNumberHistoryItems());
        connectDialog.setLocationRelativeTo(connectButton);
        
        JPanel southPanel = new JPanel(new BorderLayout());
        JPanel southPanelEast = new JPanel(new BorderLayout());
        southPanelEast.setBorder(new LineBorder(southPanelEast.getBackground(), 1));
        
        internalLogMessageLabel = new InternalLogLabel();
        internalLogDialog = new InternalLogDialog(dialogContainer);
        addInternalLogListener(internalLogMessageLabel);
        addInternalLogListener(internalLogDialog);
        
                
        packetCounterTextField = new JTextField("0", 8);       
        packetCounterTextField.setBackground(Color.BLACK);
        packetCounterTextField.setForeground(ErrorColorTable.INFO_COLOR);
        packetCounterTextField.setHorizontalAlignment(JTextField.LEFT);
        packetCounterTextField.setBorder(null);
        
        packetCounterLabel = new JLabel("N Packets=");
        packetCounterLabel.setOpaque(true);
        packetCounterLabel.setBackground(Color.BLACK);
        packetCounterLabel.setForeground(ErrorColorTable.INFO_COLOR);

        searchField = new JTextField(10);
        searchField.setOpaque(true);
        searchField.setBackground(Color.WHITE);
        searchField.setForeground(Color.BLACK);
        searchField.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                handleSearchTxtFieldRequest(e);
            }
        });

        searchField.addKeyListener(new KeyListener() {

            public void keyTyped(KeyEvent e) {                
            }

            public void keyPressed(KeyEvent e) {                
            }

            public void keyReleased(KeyEvent e) {
                handleSearchKeyReleased(e);
            }
        });

        southPanelEast.add(packetCounterLabel, BorderLayout.CENTER);
        southPanelEast.add(packetCounterTextField, BorderLayout.EAST);
        southPanelEast.add(searchField, BorderLayout.WEST);
        
        southPanel.add(internalLogMessageLabel, BorderLayout.CENTER);
        southPanel.add(southPanelEast, BorderLayout.EAST);
        
        if(mainContainer instanceof JFrame){
            ((JFrame)mainContainer).addWindowListener(new WindowAdapter() {
                @Override
                public void windowClosing(WindowEvent evt)
                {
                    handleWindowClosing(evt);
                }
            });
        }
        
        mainContainer.add(splitPane, BorderLayout.CENTER);
        mainContainer.add(toolbar, BorderLayout.NORTH);
        mainContainer.add(southPanel, BorderLayout.SOUTH);
        mainContainer.setVisible(true);
        mainContainer.setPreferredSize(jtPrefs.getPreferredDimension());
        if(mainContainer instanceof Window){
            ((Window)mainContainer).pack();
        }
        internalLogDialog.pack();        
        
        //Add key binding for search
        if(mainContainer instanceof RootPaneContainer){
            ((RootPaneContainer)mainContainer).getRootPane().getInputMap().put(KeyStroke.getKeyStroke(
                                            KeyEvent.VK_F, InputEvent.CTRL_DOWN_MASK),
                                            "searchField");
            ((RootPaneContainer)mainContainer).getRootPane().getActionMap().put("searchField", new AbstractAction() {
                public void actionPerformed(ActionEvent e) {
                    searchField.requestFocus();
                }
            });
        }
        loggerTree.getInputMap().put(KeyStroke.getKeyStroke(
                                            KeyEvent.VK_F, InputEvent.CTRL_DOWN_MASK),
                                            "searchField");
        loggerTree.getActionMap().put("searchField", new AbstractAction() {
            public void actionPerformed(ActionEvent e) {
                searchField.requestFocus();
            }
        });
        ToolTipManager.sharedInstance().registerComponent(loggerTree);
        fireNewInternalLogMessage(InternalLogErrorLevel.INFO, "Welcome to JTLogger");                
    }
    
    private class UDPRemoteConnectionThread extends Thread{
        boolean starting = true;
        
        @Override
        public void run(){
            while(UDPConnectorRunning){
                try{
                    String[] addrStr = connectDialog.getRemoteHost().split(":");
                    int remotePort = 0;
                    if(addrStr.length != 2){
                        break;
                    }
                    try{
                        remotePort = Integer.parseInt(addrStr[1]);
                    }
                    catch(NumberFormatException nfe){
                        nfe.printStackTrace();
                        break;
                    }
                    
                    InetAddress remoteAddress = InetAddress.getByName(addrStr[0]);
                    DatagramSocket socket = new DatagramSocket();

                    String toSend = "" + connectDialog.getLocalPort();
                    if(starting && connectDialog.isUseHistory()){
                        toSend += "|" + connectDialog.getNHistoryItems() + "|1";
                        starting = false;
                    }

                    DatagramPacket dp = new DatagramPacket(toSend.getBytes(), toSend.length(), remoteAddress, remotePort);
                    socket.send(dp);
                    socket.close();
                    
                    try{
                        synchronized(UDPConnectorSync){
                            UDPConnectorSync.wait(10000);
                        }
                    }catch(InterruptedException ie){
                        ie.printStackTrace();
                    }
                }
                catch(Exception e){
                    e.printStackTrace();
                }
            }
        }
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        new JTLogger(new JFrame("JTLogger"));
    }
    
    public void newEntriesAvailables(Vector<DBEntry> list) {
        packetCounter+=list.size();
        packetCounterTextField.setText("" + packetCounter);
    }
    
    public void serverStatusChanged(ServerState newState, String extraInfo) {
        switch(newState){
            case STARTING:               
                fireNewInternalLogMessage(InternalLogErrorLevel.INFO, "UDP Server is starting in port: " + connectDialog.getLocalPort());
                break;
            case STARTED:
                disconnectButton.setEnabled(true);
                fireNewInternalLogMessage(InternalLogErrorLevel.INFO, "UDP Server Started in port: " + connectDialog.getLocalPort());
                break;
            case STOPPING:
                fireNewInternalLogMessage(InternalLogErrorLevel.WARNING, "Waiting for UDP server to shutdown");
                break;
            case STOPPED:
                connectButton.setEnabled(true);
                fireNewInternalLogMessage(InternalLogErrorLevel.INFO, "UDP server is shutdown");
                break;
            case ERROR:
                connectButton.setEnabled(true);
                disconnectButton.setEnabled(false);
                fireNewInternalLogMessage(InternalLogErrorLevel.ERROR, "UDP server error: " + extraInfo);
                break;
        }
    }
    
    public void preferencesChanged() {
        packetCounterLabel.setBackground(JTPreferences.getJTPreferences().getBackgroundColor());
        packetCounterTextField.setBackground(JTPreferences.getJTPreferences().getBackgroundColor());
        loggerTextScrollPane.getViewport().setBackground(JTPreferences.getJTPreferences().getBackgroundColor());
    }

    protected void handleWindowClosing(WindowEvent evt){
        savePrefs();
        server.shutdown();
        clearTempFiles();
        ddbInterface.shutdown();
        clearTempFiles();
        if(mainContainer instanceof JFrame){
            System.exit(0);
        }
    }
    
    private void savePrefs(){
        jtPrefs.setPreferredDimension(mainContainer.getSize());
        jtPrefs.setLocalPort(connectDialog.getLocalPort());
        jtPrefs.setRemoteHost(connectDialog.getRemoteHost());
        jtPrefs.setUseHistory(connectDialog.isUseHistory());
        jtPrefs.setNumberHistoryItems(connectDialog.getNHistoryItems());
    }
    
    private void handleConnectionRequest(ActionEvent evt){
        connectDialog.setVisible(true);        
        if(connectDialog.okSelected()){
            startConnections();
        }
    }
    
    private void startConnections(){
        connectButton.setEnabled(false);
        disconnectButton.setEnabled(false);
        packetCounter = 0;
        boolean startOK = server.start(connectDialog.getLocalPort());
        if(startOK){
            UDPConnectorRunning = true;
            new UDPRemoteConnectionThread().start();
        }
    }
    
    private void handledisconnectRequest(ActionEvent evt){        
        connectButton.setEnabled(false);
        disconnectButton.setEnabled(false);
        server.shutdown();        
        synchronized(UDPConnectorSync){
            UDPConnectorRunning = false;
            UDPConnectorSync.notifyAll();
        }
    }
    
    
    private void handleClearLog(ActionEvent evt){
    	loggerText.setText("");
    }
    
    
    private void handledInternalLoggerButtonRequest(ActionEvent evt){    
        internalLogDialog.setVisible(true);
    }
    
    private void handledPrefsDialogButtonRequest(ActionEvent evt){
        if(!prefsDialog.isVisible())
            prefsDialog.setVisible(true);
    }

    private void handleSearchTxtFieldRequest(ActionEvent evt){
        loggerText.search(searchField.getText().trim());
    }

    private void handleSearchKeyReleased(KeyEvent e){
        loggerText.search(searchField.getText().trim());
    }
    
    //List of listeners for the internal messages logging
    private EventListenerList listenersList = new EventListenerList();
    
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
    
    public void addInternalLogListener(InternalLogListener listener){
        listenersList.add(InternalLogListener.class, listener);
    }
    
    public void removeInternalLogListener(InternalLogListener listener){
        listenersList.remove(InternalLogListener.class, listener);
    }
    
    private void clearTempFiles(){
        File tmpDir = new File(JTPreferences.logDirectory);
        File[] list = tmpDir.listFiles();
        for(int i=0; i<list.length; i++){
            list[i].delete();
        }
    }
}
