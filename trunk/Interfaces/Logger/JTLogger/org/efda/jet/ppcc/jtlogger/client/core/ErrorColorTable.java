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
import java.util.Hashtable;

/**
 *
 * @author andre
 */
public abstract class ErrorColorTable {
    private static Hashtable<Integer, Color> errorColorCode = new Hashtable<Integer, Color>();
    
    public static int NEGATIVE_ERRORS_COLOR_CODE = Integer.MIN_VALUE;
    public static int POSITIVE_ERRORS_COLOR_CODE = Integer.MAX_VALUE;
    
    public static Color INFO_COLOR = Color.GREEN;
    public static Color ERROR_COLOR = Color.RED;
    public static Color WARNING_COLOR = Color.ORANGE;
    
    static{        
        errorColorCode.put(0, WARNING_COLOR);
        errorColorCode.put(-1, ERROR_COLOR);
        errorColorCode.put(-2, Color.YELLOW);
        errorColorCode.put(-3, new Color(139, 0, 139));
        errorColorCode.put(1, INFO_COLOR);
        errorColorCode.put(NEGATIVE_ERRORS_COLOR_CODE, Color.RED);
        errorColorCode.put(POSITIVE_ERRORS_COLOR_CODE, new Color(153, 0, 0));
    }
    
    public static Color getColor(int errorCode){
        Color c = errorColorCode.get(errorCode);
        if(c == null){
            if(errorCode < 0){
                c = errorColorCode.get(NEGATIVE_ERRORS_COLOR_CODE);
            }
            else{
                c = errorColorCode.get(POSITIVE_ERRORS_COLOR_CODE);
            }
        }
        return c;
    }
    
    public static void updateColorTable(int[] codes, Color[] values){
        if(codes.length != values.length)
            return;
        for(int i=0; i<codes.length; i++){
            errorColorCode.put(codes[i], values[i]);
        }
    }
}
