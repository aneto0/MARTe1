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
package org.efda.jet.ppcc.jtlogger.client.ui;

import java.awt.Color;
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.io.Reader;
import java.io.StringReader;
import java.util.concurrent.Semaphore;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.EditorKit;
import javax.swing.text.html.HTMLDocument;
import org.efda.jet.ppcc.jtlogger.client.core.DBInterface;
import org.efda.jet.ppcc.jtlogger.client.core.ErrorStringTable;
import org.efda.jet.ppcc.jtlogger.client.core.JTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.KeyType;
import org.efda.jet.ppcc.jtlogger.client.core.LoggerFilterListener;
import org.efda.jet.ppcc.jtlogger.client.core.PreferenceChangeListener;

/**
 *
 * @author andre
 */
public class LoggerTextPane extends JTextPane implements LoggerFilterListener, PreferenceChangeListener {

    private JScrollPane containerPane = null;
    private HTMLDocument styleDoc = null;
    private DBInterface dbInterface = null;
    private long currentKey = 0;
    private KeyType[] currentTypes;
    private int[] currentValues;
    private static String LS = System.getProperty("line.separator");
    private int maxSize = 10000;
    private String logDirectory = JTPreferences.logDirectory;
    private EditorKit kit = null;
    private Tail tail;
    private WordSearcher wordSearcher = null;
    private String searchText = "";

    public LoggerTextPane(DBInterface dbInterface) {
        super();

        this.dbInterface = dbInterface;

        setContentType("text/html");
        styleDoc = new HTMLDocument();
        setDocument(styleDoc);
        setDoubleBuffered(true);

        setEditable(false);

        kit = getEditorKit();

        setBackground(Color.BLACK);

        tail = new Tail();
        tail.start();
        tail.changeFile();

        wordSearcher = new WordSearcher(this);
    }

    public void setContainerPane(JScrollPane containerPane) {
        this.containerPane = containerPane;
    }

    public void filter(long key, KeyType[] type, int[] values) {
        this.currentKey = key;
        this.currentTypes = type;
        this.currentValues = values;
        tail.reset();
        tail.changeFile();
    }

    private boolean exists(String entry) {
        if (currentKey == 0) {
            return true;
        }

        for (int i = 0; i < currentTypes.length; i++) {
            switch (currentTypes[i]) {
                case THREADS:
                    String tid = String.format(":tid=0x%x", currentValues[i]);
                    if (entry.indexOf(tid) == -1) {
                        return false;
                    }
                    break;
                case ERROR:
                    String error = ":" + ErrorStringTable.getError(currentValues[i]) + ":";
                    if (entry.indexOf(error) == -1) {
                        return false;
                    }
                    break;
                case CLASSES:
                    String cid = String.format(":cid=0x%x:", currentValues[i]);
                    if (entry.indexOf(cid) == -1) {
                        return false;
                    }
                    break;
                case OBJECTS:
                    String oid;
                    if (currentValues[i] == 0) {
                        oid = "GLOBAL";
                    } else {
                        oid = String.format("0x%x:", currentValues[i]);
                    }

                    if (entry.indexOf(oid) == -1) {
                        return false;
                    }
                    break;
            }
        }

        return true;
    }

    private void append(String line) {
        try {
            Reader r = new StringReader(line);
            kit.read(r, styleDoc, styleDoc.getLength());
        } catch (IOException ioe) {
            UIManager.getLookAndFeel().provideErrorFeedback(LoggerTextPane.this);
        } catch (BadLocationException ble) {
            UIManager.getLookAndFeel().provideErrorFeedback(LoggerTextPane.this);
        }
    }

    private class Tail extends Thread {

        private RandomAccessFile ras = null;
        private String line;
        private int checkCounter = 0;
        private boolean changed = true;
        private StringBuffer buffer = null;

        public Tail(){            
        }

        public void run() {
            while (true) {
                try {
                    if (ras != null) {
                        line = ras.readLine();
                        if (line != null) {
                            if (exists(line)) {
                                if (changed) {
                                    if (buffer == null) {
                                        buffer = new StringBuffer(1024);
                                    }
                                    buffer.append(line + LS);
                                } else {
                                    append(line);
                                    moveScrollToBottom();
                                }
                            }
                            continue;
                        }                        
                    }
                    if (changed && buffer != null) {
                        HTMLDocument styleDocTemp = new HTMLDocument();
                        try {
                            Reader r = new StringReader(buffer.toString());
                            kit.read(r, styleDocTemp, 0);
                        } catch (IOException ioe) {
                            UIManager.getLookAndFeel().provideErrorFeedback(LoggerTextPane.this);
                        } catch (BadLocationException ble) {
                            UIManager.getLookAndFeel().provideErrorFeedback(LoggerTextPane.this);
                        }
                        styleDoc = styleDocTemp;
                        setDocument(styleDoc);
                        moveScrollToBottom();
                        changed = false;
                        buffer = null;                        
                    }

                    sleep(150);
                    if ((checkCounter++) == 200) {
                        checkMaxSize();
                        checkCounter = 0;
                    }                    
                } catch (Exception ioe) {
                    ioe.printStackTrace();
                }
            }
        }

        public void reset() {
            try {
                styleDoc.remove(0, styleDoc.getLength());
            } catch (Exception e) {
                e.printStackTrace();
                reset();
            }
        }

        public void changeFile() {
            try {
                String filename = JTPreferences.allEntriesFile;
                if (currentKey != 0) {
                    filename = "A" + currentKey + ".log";
                }
                RandomAccessFile newRas = new RandomAccessFile(new File(logDirectory, filename), "r");
                long seek = newRas.length() - maxSize;
                if (seek < 0) {
                    seek = 0;
                }
                newRas.seek(seek);

                if (ras != null) {
                    ras.close();
                }
                ras = newRas;
                changed = true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public void shutdown() {
            if (ras != null) {
                try {
                    ras.close();
                } catch (IOException ioe) {
                    ioe.printStackTrace();
                }
            }
        }

        private void checkMaxSize() {
            if (styleDoc.getLength() > maxSize) {
                int delta = styleDoc.getLength() - maxSize;
                try {
                    styleDoc.remove(0, delta);
                } catch (Throwable e) {
                    checkMaxSize();
                    e.printStackTrace();
                }
            }
        }
    }

    private void moveScrollToBottom() {
        SwingUtilities.invokeLater(new Runnable() {

            public void run() {
                containerPane.getVerticalScrollBar().setValue(getDocument().getLength());
            }
        });
    }

    public void preferencesChanged() {
        setBackground(JTPreferences.getJTPreferences().getBackgroundColor());
        maxSize = JTPreferences.getJTPreferences().getMaxPackets() * 20;
    }

    @Override
    public void finalize() {
        tail.shutdown();
    }

    public int getLength(){
        return getDocument().getLength();
    }

    public void search(String s) {
        this.searchText = s;
        int offset = wordSearcher.search(s);
        if (offset != -1) {
            try {
                scrollRectToVisible(modelToView(offset));
            } catch (BadLocationException e) {
            }
        }
    }    
}
