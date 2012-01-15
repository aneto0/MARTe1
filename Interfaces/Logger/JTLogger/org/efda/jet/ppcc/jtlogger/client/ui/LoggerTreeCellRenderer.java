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
import java.awt.Component;
import java.awt.Font;
import java.util.GregorianCalendar;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import org.efda.jet.ppcc.jtlogger.client.core.DBEntry;
import org.efda.jet.ppcc.jtlogger.client.core.JTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.KeyType;

/**
 *
 * @author andre
 */
public class LoggerTreeCellRenderer extends DefaultTreeCellRenderer{
    
     /**Unselected background color*/
    public static final Color UNSELECTED_COLOR = UIManager.getColor("Tree.textBackground");
    
    /**Selected background color*/
    public static final Color SELECTED_COLOR = UIManager.getColor("Tree.selectionBackground");
    
    /**The icon set*/
    private ImageIcon LOGGER_ICON = null;
    private ImageIcon ERROR_ICON = null;
    private ImageIcon OBJECT_ICON = null;
    private ImageIcon CLASS_ICON = null;    
    private ImageIcon THREAD_ICON = null;
    private ImageIcon LOG_SOURCE_ICON = null;
    
    public LoggerTreeCellRenderer(){
        super();
        LOGGER_ICON = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/logger.png"));
        ERROR_ICON = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/error.png"));
        OBJECT_ICON = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/object.png"));
        CLASS_ICON = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/class.png"));
        THREAD_ICON = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/thread.png"));
        LOG_SOURCE_ICON = new ImageIcon(getClass().getResource("/org/efda/jet/ppcc/jtlogger/client/resources/logsource.png"));
    }
        
    @Override
    public Component getTreeCellRendererComponent(JTree tree, Object val, boolean sel, boolean expanded, boolean leaf, int row, boolean hasFocus)
    {        
        super.getTreeCellRendererComponent(tree, val, sel, expanded, leaf, row, hasFocus);
                
        Component c = null;
        
        JLabel label = new JLabel();
        String txt = null;
        DefaultMutableTreeNode dmtn = (DefaultMutableTreeNode)val;
        int level = dmtn.getLevel();
        Object value = dmtn.getUserObject();
        
        if(value instanceof DBEntry){
            DBEntry entry = (DBEntry)value;
            switch(level){
                case 1:
                    String taskName = "";
                    if(entry.getTaskName() == null)
                        taskName = "0x" + String.format("%x", entry.getTaskID());
                    txt = entry.getIPName() + " - " + taskName;
                    label.setIcon(LOG_SOURCE_ICON);                    
                                        
                    label.setToolTipText("Created: " + entry.getPrettyCreationTime());
                    if(((System.currentTimeMillis() / 1000) - entry.getLastPingTime()) > JTPreferences.getJTPreferences().getNodeObsoleteSeconds()){
                        label.setForeground(Color.RED);
                    }
                    break;
                case 3:
                case 5:
                    TextNode tn = (TextNode)((DefaultMutableTreeNode)(dmtn.getParent())).getUserObject();
                    if(tn.getType() == KeyType.THREADS){
                        txt = entry.getThreadName();
                        if(txt == null || txt.length() == 0){
                            txt = "0x" + String.format("%x", entry.getThreadID());
                        }                        
                    }
                    else if(tn.getType() == KeyType.ERROR){
                        txt = "" + entry.getErrorCode();
                        if(entry.getErrorName() != null)
                            txt += " - " + entry.getErrorName();                        
                    }
                    else if(tn.getType() == KeyType.CLASSES){
                        if(entry.getClassID() == 0){
                            txt = "GLOBAL";
                        }
                        else{
                            txt = "0x" + String.format("%x", entry.getClassID());
                            if(entry.getClassName() != null && entry.getClassName().length() > 0){
                                txt += " - " + entry.getClassName();
                            }
                        }                        
                    }
                    else if(tn.getType() == KeyType.OBJECTS){
                        if(entry.getObjectAddress() == 0){
                            txt = "GLOBAL";
                        }
                        else{
                            txt = "0x" + String.format("%x", entry.getObjectAddress());
                            if(entry.getObjectName() != null && entry.getObjectName().length() > 0)
                                txt += " - " + entry.getObjectName();
                        }
                    }
                    break;
            }            
            //txt += " :: " + entry.getUniqueID();            
        }
        else if(value instanceof TextNode){
            txt = value.toString();
            KeyType type = ((TextNode)value).getType();
            switch(type){
                case THREADS:
                    label.setIcon(THREAD_ICON);
                    break;
                case ERROR:
                    label.setIcon(ERROR_ICON);
                    break;
                case CLASSES:
                    label.setIcon(CLASS_ICON);
                    break;
                case OBJECTS:
                    label.setIcon(OBJECT_ICON);
                    break;                    
            }            
        }
        
        
        label.setText(txt);
        label.setFont(label.getFont().deriveFont(Font.PLAIN));        
        label.setOpaque(true);
        
        if(level == 0){         
            label.setIcon(LOGGER_ICON);
        }
        
        if(selected)
            label.setBackground(SELECTED_COLOR);
        else
            label.setBackground(UNSELECTED_COLOR);
        
        return label;
    }
}
