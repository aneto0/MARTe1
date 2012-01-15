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
import java.util.GregorianCalendar;
import javax.swing.JLabel;
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
public class InternalLogLabel extends JLabel implements InternalLogListener, PreferenceChangeListener{

    public InternalLogLabel(){
        setOpaque(true);
        setBackground(Color.BLACK);
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
        setForeground(color);
        setText("[" + date + "] " + message);
    }

    public void preferencesChanged() {
        setBackground(JTPreferences.getJTPreferences().getBackgroundColor());
    }
}
