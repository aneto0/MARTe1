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

import java.util.Hashtable;

/**
 *
 * @author andre
 */
public abstract class ErrorStringTable {

    private static Hashtable<Integer, String> errorStringCode = new Hashtable<Integer, String>();
    
    static{
        
        errorStringCode.put(0, "Warning");
        errorStringCode.put(1, "Information");
        errorStringCode.put(-1, "FatalError");
        errorStringCode.put(-2, "RecoverableError");
        errorStringCode.put(-3, "InitialisationError");
        errorStringCode.put(-4, "OSError");
        errorStringCode.put(-5, "ParametersError");
        errorStringCode.put(-6, "IllegalOperation");
        errorStringCode.put(-7, "ErrorSharing");
        errorStringCode.put(-8, "ErrorAccessDenied");
        errorStringCode.put(-9, "Exception");
        errorStringCode.put(-10, "Timeout");
        errorStringCode.put(-11, "CommunicationError");
        errorStringCode.put(-12, "SyntaxError");
    }
    
    public static String getError(int errorCode){
        return errorStringCode.get(errorCode);
    }
    
    public static void updateErrorTable(int[] codes, String[] values){
        if(codes.length != values.length)
            return;
        for(int i=0; i<codes.length; i++){
            errorStringCode.put(codes[i], values[i]);
        }
    }
}
