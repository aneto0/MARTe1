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
import java.awt.Dimension;
import java.awt.Frame;
import java.util.GregorianCalendar;
import javax.swing.JDialog;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.SwingUtilities;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;
import org.efda.jet.ppcc.jtlogger.client.core.ErrorColorTable;
import org.efda.jet.ppcc.jtlogger.client.core.IJTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.InternalLogErrorLevel;
import org.efda.jet.ppcc.jtlogger.client.core.InternalLogListener;
import org.efda.jet.ppcc.jtlogger.client.core.JTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.PreferenceChangeListener;

/**
 *
 * @author andre
 */
public class InternalLogDialog extends JDialog implements InternalLogListener, PreferenceChangeListener{

    private JTextPane logPane = null;
    
    private static String LS = System.getProperty("line.separator");
    private SimpleAttributeSet msgAttrError;
    
    private JScrollPane containerPane = null;
    
    private StyledDocument styleDoc = null;
    
    
    public InternalLogDialog(Frame owner){
        super(owner);
        setTitle("JTL Intenal Logger");
        setModal(false);
        setAlwaysOnTop(true);
                        
        logPane = new JTextPane();
        logPane.setEditable(false);
        logPane.setBackground(Color.BLACK);
        msgAttrError = new SimpleAttributeSet();
        styleDoc = logPane.getStyledDocument();
        
        containerPane = new JScrollPane(logPane);
        containerPane.setPreferredSize(new Dimension(400, 400));
        getContentPane().add(containerPane);
    }

    public void newLogMessageAvailable(InternalLogErrorLevel level, long time, String message) {
        Color color = Color.BLACK;
        GregorianCalendar gc = new GregorianCalendar();
        gc.setTimeInMillis(time);
        String date = "" + gc.get(GregorianCalendar.HOUR_OF_DAY);
        date += ":" + gc.get(GregorianCalendar.MINUTE);
        date += ":" + gc.get(GregorianCalendar.SECOND);
        switch(level){
            case WARNING:
                color = ErrorColorTable.WARNING_COLOR;
                break;
            case ERROR:
                color = ErrorColorTable.ERROR_COLOR;
                break;
            case INFO:
                color = ErrorColorTable.INFO_COLOR;
                break;                
        }
        StyleConstants.setForeground(msgAttrError, color);
        StyleConstants.setBold(msgAttrError, true);
        
        try{
            styleDoc.insertString(styleDoc.getLength(), "[" + date + "] " + message + LS, msgAttrError);            
        }
        catch(Throwable e){
            newLogMessageAvailable(level, time, message);
            e.printStackTrace();
        }
        
        moveScrollToBottom();
    }
    
    public void preferencesChanged() {
        logPane.setBackground(JTPreferences.getJTPreferences().getBackgroundColor());        
    }
    
    private void moveScrollToBottom(){
        SwingUtilities.invokeLater(new Runnable()
        {
            public void run()
            {                
                containerPane.getVerticalScrollBar().setValue(styleDoc.getLength());
            }
        });
    }

}
