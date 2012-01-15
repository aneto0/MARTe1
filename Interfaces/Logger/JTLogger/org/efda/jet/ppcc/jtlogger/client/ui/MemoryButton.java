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
import java.awt.Graphics;
import java.text.DecimalFormat;
import java.util.ResourceBundle;
import javax.swing.JButton;
import javax.swing.UIManager;

/**
 *
 * @author andre
 */
public class MemoryButton extends JButton
{
    /**When should the memory be updated?*/
    private long updateTime = 2000;
    
    /**The text format*/
    private DecimalFormat df = new DecimalFormat("0.0");
    
    /**Background color*/
    private Color bgColor = UIManager.getColor("ProgressBar.selectionBackground");
    
    /**Background color with alpha*/
    private Color bgColorAlpha = new Color(bgColor.getRed(), bgColor.getGreen(), bgColor.getBlue(), 80);
            
    /**The system free memory in MB*/
    private double usedMem = 0;
    
    /**The system total memory in MB*/
    private double totalMem = 0;
    
    /**The width in pixels of each updateUnit*/
    private int updateUnitWidth = 5;
    
    /**Memory history*/
    private double[] memHistory = new double[10];
    
    /**The system runtime*/
    private Runtime runtime = Runtime.getRuntime();
    
    /**The system max memory in MB*/
    private double maxMem = runtime.maxMemory() / Math.pow(1024,2);
    
    
    /** Creates a new instance of MemoryButton */
    public MemoryButton(long updateTime)
    {
        this.updateTime = updateTime;
        
        addActionListener(new java.awt.event.ActionListener()
        {
            public void actionPerformed(java.awt.event.ActionEvent evt)
            {
                Runtime.getRuntime().gc();
                setText("GC!");
            }
        });
        
        /*String tip = ResourceBundle.getBundle("org/cfn/scad/gui/other/MemoryButtonBundle").getString("forcegc") + 
                     ResourceBundle.getBundle("org/cfn/scad/gui/other/MemoryButtonBundle").getString("maxmemory") +
                     " " + df.format(maxMem) + " MB"; 
        setToolTipText(tip);*/
        new MemoryUpdater().start();
    }
    
    class MemoryUpdater extends Thread
    {
        public void run()
        {
            while(true)
            {
                try
                {
                    totalMem = runtime.totalMemory()/Math.pow(1024,2);
                    usedMem = totalMem - runtime.freeMemory()/Math.pow(1024,2);
                    
                    updateMem();
                    
                    sleep(updateTime);
                }
                catch(Exception e)
                {
                    e.printStackTrace();
                }
            }
        }
    }
    
    private void updateMem()
    {
        setText(df.format(usedMem) + "/" + df.format(totalMem) + " MB");
        
        for(int i=0; i<memHistory.length - 1; i++)
            memHistory[i] = memHistory[i + 1];
        
        memHistory[memHistory.length - 1] = usedMem;
        
        repaint();
    }
    
    public void setText(String text)
    {
        super.setText(text);
    }
    
    public void paintComponent(Graphics g)
    {
        super.paintComponent(g);
        
        g.setColor(bgColorAlpha);
        
        int widthU = getWidth() / memHistory.length;
        int height = 0;
        
        for(int i=0; i<memHistory.length; i++)
        {
            height = (int)(memHistory[i] * getHeight() / totalMem);
            g.fillRect(i * widthU, getHeight() - height, widthU, height);
        }
    }
}
