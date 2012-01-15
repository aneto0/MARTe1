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

import org.efda.jet.ppcc.jtlogger.client.core.KeyType;

/**
 *
 * @author andre
 */
public class TextNode {    
    
    private KeyType type;
        
    private long key;
        
    public TextNode(KeyType type){
        this.type = type;
    }
    
    public TextNode(KeyType type, long key){
        this(type);
        this.key = key;
    }
    
    @Override
    public String toString(){
        if(type == KeyType.THREADS){
            return "Threads";
        }
        else if(type == KeyType.ERROR){
            return "Errors";
        }
        else if(type == KeyType.CLASSES){
            return "Class";
        }
        else if(type == KeyType.OBJECTS){
            return "Object";
        }
        else if(type == KeyType.MAIN_KEY){
            return "Loggers";
        }
        
        return "Display type not found!";
    }
    
    public KeyType getType(){
        return type;
    }
    
    public long getMainKey(){
        return key;
    }
}
