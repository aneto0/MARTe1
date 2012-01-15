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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.lang.reflect.InvocationTargetException;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;
import javax.swing.event.EventListenerList;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreePath;
import org.efda.jet.ppcc.jtlogger.client.core.DBEntriesRemovedListener;
import org.efda.jet.ppcc.jtlogger.client.core.DBEntry;
import org.efda.jet.ppcc.jtlogger.client.core.JTPreferences;
import org.efda.jet.ppcc.jtlogger.client.core.LoggerFilter;
import org.efda.jet.ppcc.jtlogger.client.core.LoggerEntryListener;
import org.efda.jet.ppcc.jtlogger.client.core.KeyType;
import org.efda.jet.ppcc.jtlogger.client.core.LoggerFilterListener;

/**
 *
 * @author andre
 * 
 */
public class LoggerTree extends JTree implements LoggerEntryListener, DBEntriesRemovedListener {

    //The tree model
    private DefaultTreeModel model = null;
    //Root node which contains all the nodes
    private DefaultMutableTreeNode rootNode = null;
    //The filter interface. It is used to safely remove nodes
    private LoggerFilter filterInterface = null;
    //The last values record... so we don't keep asking to filter something which is already filtered
    private long lastKey = 0;
    private int[] lastValues = new int[0];
    private KeyType[] lastTypes = new KeyType[0];
    //The values to delete since they were removed from the database
    private Vector<DBEntry> toDeleteEntries = null;
    //The JPopUpMenu
    private JPopupMenu popupMenu = null;
    //Store the current keys
    private Hashtable<Long, DefaultMutableTreeNode> mainKeys = new Hashtable<Long, DefaultMutableTreeNode>();
    private Hashtable<Long, Vector<Integer>> errorKeys = new Hashtable<Long, Vector<Integer>>();

    private JTPreferences prefs = JTPreferences.getJTPreferences();
    
    public LoggerTree(LoggerFilter filterInterface) {
        this.filterInterface = filterInterface;

        toDeleteEntries = new Vector<DBEntry>();
        rootNode = new DefaultMutableTreeNode(new TextNode(KeyType.MAIN_KEY));
        model = new DefaultTreeModel(rootNode);
        setModel(model);
        addMouseListener(new MouseAdapter() {

            @Override
            public void mousePressed(MouseEvent evt) {
                handleMousePressed(evt);
            }
        });

        popupMenu = new JPopupMenu();
        JMenuItem deleteItem = new JMenuItem("Delete");
        deleteItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_DELETE, 0));
        deleteItem.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                handledDeleteItem(e);
            }
        });
        popupMenu.add(deleteItem);
        JMenuItem resetItem = new JMenuItem("Reset obsolete timer");
        resetItem.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                handledResetItem(e);
            }
        });
        popupMenu.add(resetItem);

        new ObsoleteSearcher().start();
    }

    private class ObsoleteSearcher extends Thread {

        private Vector<DefaultMutableTreeNode> toDeleteNodes = new Vector<DefaultMutableTreeNode>();
        
        @SuppressWarnings("unchecked")
        @Override
        public void run() {                        
            Enumeration<DefaultMutableTreeNode> nodes = null;
            DefaultMutableTreeNode currentNode = null;
            DBEntry entry;
            while (true) {
                try {
                    sleep(300000);
                } catch (InterruptedException ie) {
                }
                nodes = rootNode.preorderEnumeration();
                while (nodes.hasMoreElements()) {
                    currentNode = nodes.nextElement();

                    if (currentNode.getLevel() == 1) {
                        try {
                            entry = (DBEntry)currentNode.getUserObject();
                            if(((System.currentTimeMillis() / 1000) - entry.getLastPingTime()) > prefs.getNodeObsoleteSeconds()){
                                if(prefs.deleteObsoleteNodesAutomatically()){
                                    toDeleteNodes.add(currentNode);
                                }
                                else{
                                    model.nodeChanged(currentNode);
                                }
                            }                            
                        } catch (Throwable t) {
                            t.printStackTrace();
                        }
                    }
                }

                for(int i=0; i<toDeleteNodes.size(); i++){
                    deleteNode(toDeleteNodes.get(i));
                }
                
                toDeleteNodes.clear();                
            }
        }
    }

    private void handledDeleteItem(ActionEvent e) {
        if (getSelectionPath() == null) {
            return;
        }

        Object selected = getSelectionPath().getLastPathComponent();
        DefaultMutableTreeNode dmtn = null;
        dmtn = (DefaultMutableTreeNode) selected;
        if (dmtn.getLevel() != 1) {
            return;
        }

        deleteNode(dmtn);
    }
    
    private void deleteNode(DefaultMutableTreeNode dmtn){
        try {
            long key = ((DBEntry) dmtn.getUserObject()).getKey();
            mainKeys.remove(key);
            errorKeys.remove(key);
            model.removeNodeFromParent(dmtn);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private void handledResetItem(ActionEvent e) {
        if (getSelectionPath() == null) {
            return;
        }

        Object selected = getSelectionPath().getLastPathComponent();
        DefaultMutableTreeNode dmtn = null;
        dmtn = (DefaultMutableTreeNode) selected;
        if (dmtn.getLevel() != 1) {
            return;
        }

        try {
            long key = ((DBEntry) dmtn.getUserObject()).getKey();
            ((DBEntry) dmtn.getUserObject()).ping();
            model.nodeChanged(dmtn);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private void handleMousePressed(MouseEvent evt) {
        TreePath path = getClosestPathForLocation(evt.getX(), evt.getY());
        if (path == null) {
            return;
        } else {
            setSelectionPath(path);
        }

        Object selected = path.getLastPathComponent();
        DefaultMutableTreeNode selectedNode = (DefaultMutableTreeNode) selected;
        DefaultMutableTreeNode currentNode = selectedNode;
        int level = selectedNode.getLevel();

        if (evt.getButton() == MouseEvent.BUTTON3) {
            if (level == 1) {
                popupMenu.show(this, evt.getX(), evt.getY());
                return;
            }
        }


        long key = 0;
        if (level != 0) {
            while (level != 1) {
                currentNode = (DefaultMutableTreeNode) currentNode.getParent();
                level = currentNode.getLevel();
            }

            key = ((DBEntry) currentNode.getUserObject()).getKey();
        }

        currentNode = selectedNode;
        Vector<KeyType> types = new Vector<KeyType>();
        Vector<Integer> values = new Vector<Integer>();
        KeyType currentType;
        int currentValue = 0;
        Object selectObj = null;
        level = currentNode.getLevel();
        if (level > 1) {
            while (level != 2) {
                selectObj = currentNode.getUserObject();
                if (selectObj instanceof DBEntry) {
                    currentType = ((TextNode) ((DefaultMutableTreeNode) currentNode.getParent()).getUserObject()).getType();
                    types.add(currentType);
                    switch (currentType) {
                        case THREADS:
                            currentValue = ((DBEntry) selectObj).getThreadID();
                            break;
                        case ERROR:
                            currentValue = ((DBEntry) selectObj).getErrorCode();
                            break;
                        case CLASSES:
                            currentValue = ((DBEntry) selectObj).getClassID();
                            break;
                        case OBJECTS:
                            currentValue = ((DBEntry) selectObj).getObjectAddress();
                            break;
                    }
                    values.add(currentValue);
                }
                currentNode = (DefaultMutableTreeNode) currentNode.getParent();
                level = currentNode.getLevel();
            }
        }

        KeyType[] typesArr = types.toArray(new KeyType[types.size()]);
        int[] valuesArr = new int[values.size()];
        for (int i = 0; i < valuesArr.length; i++) {
            valuesArr[i] = values.get(i).intValue();
        }

        boolean changed = false;
        if (lastKey != key) {
            changed = true;
        } else if (lastValues.length != valuesArr.length) {
            changed = true;
        } else {
            boolean found = false;
            for (int i = 0; i < lastValues.length && !changed; i++) {
                for (int j = 0; j < valuesArr.length && !changed; j++) {
                    if (lastTypes[j] == typesArr[j]) {
                        changed = (valuesArr[j] != lastValues[j]);
                        found = true;
                    }
                }
                if (!found) {
                    changed = !found;
                }
            }
        }

        if (changed) {
            lastKey = key;
            lastTypes = typesArr;
            lastValues = valuesArr;
            fireFilterRequest(key, typesArr, valuesArr);
        }
    }

    private class InsertNodeThread extends Thread {

        private MutableTreeNode child = null;
        private MutableTreeNode parent = null;
        private int index = 0;

        public InsertNodeThread(MutableTreeNode child, MutableTreeNode parent, int index) {
            this.child = child;
            this.parent = parent;
            this.index = index;
        }

        @Override
        public void run() {
            model.insertNodeInto(child, parent, index);
        }
    }

    private class RemoveNodeThread extends Thread {

        private MutableTreeNode child = null;

        public RemoveNodeThread(MutableTreeNode child) {
            this.child = child;
        }

        @Override
        public void run() {
            model.removeNodeFromParent(child);
        }
    }

    public void newEntriesAvailables(Vector<DBEntry> list) {
        DefaultMutableTreeNode entryNode = null;
        long key = 0;
        int hash = 0;

        for (int i = 0; i < list.size(); i++) {
            key = list.get(i).getKey();

            hash = new String("" + list.get(i).getThreadID() + "" + list.get(i).getErrorCode() + "" + list.get(i).getObjectAddress() + "" + list.get(i).getClassID()).hashCode();
            //Check if the main key is present
            if (!mainKeys.containsKey(key)) {
                entryNode = addMainNode(list.get(i));
                mainKeys.put(key, entryNode);
                errorKeys.put(key, new Vector<Integer>());
            }

            entryNode = mainKeys.get(key);
            ((DBEntry)entryNode.getUserObject()).ping();

            if (!errorKeys.get(key).contains(hash)) {
                errorKeys.get(key).add(new Integer(hash));
                addToEntry(entryNode, list.get(i), KeyType.ERROR);
                addToEntry(entryNode, list.get(i), KeyType.THREADS);
                addToEntry(entryNode, list.get(i), KeyType.OBJECTS);
                addToEntry(entryNode, list.get(i), KeyType.CLASSES);
            }
        }
    }

    public void dbEntriesRemoved(Vector<DBEntry> entries) {
        toDeleteEntries.addAll(entries);
    }

    private DefaultMutableTreeNode addMainNode(DBEntry entry) {
        DefaultMutableTreeNode currentNode = new DefaultMutableTreeNode(entry);

        //Insert the default structure
        insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.THREADS)), currentNode, currentNode.getChildCount());
        insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.ERROR)), currentNode, currentNode.getChildCount());
        insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.CLASSES)), currentNode, currentNode.getChildCount());
        insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.OBJECTS)), currentNode, currentNode.getChildCount());

        insertNodeInto(currentNode, rootNode, rootNode.getChildCount());
        model.nodeStructureChanged(rootNode);
        return currentNode;
    }

    private void addToEntry(DefaultMutableTreeNode entryNode, DBEntry child, KeyType type) {
        DBEntry entry = null;
        boolean found = false;

        DefaultMutableTreeNode currentNode = null;
        DefaultMutableTreeNode masterNode = null;

        switch (type) {
            case THREADS:
                masterNode = (DefaultMutableTreeNode) entryNode.getChildAt(0);
                break;
            case ERROR:
                masterNode = (DefaultMutableTreeNode) entryNode.getChildAt(1);
                break;
            case CLASSES:
                masterNode = (DefaultMutableTreeNode) entryNode.getChildAt(2);
                break;
            case OBJECTS:
                masterNode = (DefaultMutableTreeNode) entryNode.getChildAt(3);
                break;
        }

        int comp1 = 0;
        int comp2 = 0;
        for (int r = 0; r < masterNode.getChildCount() && !found; r++) {
            currentNode = (DefaultMutableTreeNode) masterNode.getChildAt(r);
            entry = (DBEntry) currentNode.getUserObject();

            switch (type) {
                case THREADS:
                    comp1 = entry.getThreadID();
                    comp2 = child.getThreadID();
                    break;
                case ERROR:
                    comp1 = entry.getErrorCode();
                    comp2 = child.getErrorCode();
                    break;
                case OBJECTS:
                    comp1 = entry.getObjectAddress();
                    comp2 = child.getObjectAddress();
                    break;
                case CLASSES:
                    comp1 = entry.getClassID();
                    comp2 = child.getClassID();
                    break;
            }

            found = (comp1 == comp2);
        }

        if (!found) {
            currentNode = new DefaultMutableTreeNode(child);
            insertNodeInto(currentNode, masterNode, masterNode.getChildCount());
            //Insert the default structure            
            switch (type) {
                case THREADS:
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.ERROR)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.CLASSES)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.OBJECTS)), currentNode, currentNode.getChildCount());
                    break;
                case ERROR:
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.THREADS)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.CLASSES)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.OBJECTS)), currentNode, currentNode.getChildCount());
                    break;
                case OBJECTS:
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.THREADS)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.ERROR)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.CLASSES)), currentNode, currentNode.getChildCount());
                    break;
                case CLASSES:
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.THREADS)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.ERROR)), currentNode, currentNode.getChildCount());
                    insertNodeInto(new DefaultMutableTreeNode(new TextNode(KeyType.OBJECTS)), currentNode, currentNode.getChildCount());
                    break;
            }
        }

        //search the correct node        
        for (int i = 0; i < masterNode.getChildCount() && !found; i++) {

            currentNode = (DefaultMutableTreeNode) masterNode.getChildAt(i);
            entry = (DBEntry) currentNode.getUserObject();
            switch (type) {
                case THREADS:
                    comp1 = entry.getThreadID();
                    comp2 = child.getThreadID();
                    break;
                case ERROR:
                    comp1 = entry.getErrorCode();
                    comp2 = child.getErrorCode();
                    break;
                case OBJECTS:
                    comp1 = entry.getObjectAddress();
                    comp2 = child.getObjectAddress();
                    break;
                case CLASSES:
                    comp1 = entry.getClassID();
                    comp2 = child.getClassID();
                    break;
            }

            found = (comp1 == comp2);
        }

        if (found) {
            int count = currentNode.getChildCount();
            int subcount = 0;
            for (int i = 0; i < count; i++) {
                found = false;
                subcount = currentNode.getChildAt(i).getChildCount();
                type = ((TextNode) ((DefaultMutableTreeNode) currentNode.getChildAt(i)).getUserObject()).getType();
                for (int j = 0; j < subcount && !found; j++) {
                    entry = (DBEntry) ((DefaultMutableTreeNode) (currentNode.getChildAt(i).getChildAt(j))).getUserObject();
                    switch (type) {
                        case THREADS:
                            comp1 = entry.getThreadID();
                            comp2 = child.getThreadID();
                            break;
                        case ERROR:
                            comp1 = entry.getErrorCode();
                            comp2 = child.getErrorCode();
                            break;
                        case OBJECTS:
                            comp1 = entry.getObjectAddress();
                            comp2 = child.getObjectAddress();
                            break;
                        case CLASSES:
                            comp1 = entry.getClassID();
                            comp2 = child.getClassID();
                            break;
                    }

                    found = (comp1 == comp2);
                }
                if (!found) {
                    entryNode = new DefaultMutableTreeNode(child);
                    insertNodeInto(entryNode, (DefaultMutableTreeNode) currentNode.getChildAt(i), currentNode.getChildAt(i).getChildCount());
                }
            }
        }
    }
    private EventListenerList listenersList = new EventListenerList();

    private void fireFilterRequest(long key, KeyType[] type, int[] value) {
        if (listenersList == null) {
            return;
        }
        Object[] listeners = listenersList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i] == LoggerFilterListener.class) {
                ((LoggerFilterListener) listeners[i + 1]).filter(key, type, value);
            }
        }
    }

    public void addLoggerFilterListener(LoggerFilterListener listener) {
        listenersList.add(LoggerFilterListener.class, listener);
    }

    public void removeLoggerFilterListener(LoggerFilterListener listener) {
        listenersList.remove(LoggerFilterListener.class, listener);
    }

    private void insertNodeInto(MutableTreeNode child, MutableTreeNode parent, int index) {
        try {
            SwingUtilities.invokeAndWait(new InsertNodeThread(child, parent, index));
        } catch (InterruptedException ie) {
            ie.printStackTrace();
        } catch (InvocationTargetException ite) {
            ite.printStackTrace();
        }
    }

    private int getKeyTypeValue(DefaultMutableTreeNode node) {
        if (node.getLevel() != 1 && node.getLevel() != 3 && node.getLevel() != 5) {
            return -1;
        }
        DBEntry currentEntry = (DBEntry) node.getUserObject();
        if (node.getLevel() == 1) {
            return (int) currentEntry.getKey();
        }

        KeyType type = getType(node);
        int value = 0;
        switch (type) {
            case THREADS:
                value = currentEntry.getThreadID();
                break;
            case ERROR:
                value = currentEntry.getErrorCode();
                break;
            case CLASSES:
                value = currentEntry.getClassID();
                break;
            case OBJECTS:
                value = currentEntry.getObjectAddress();
                break;
        }

        return value;
    }

    private KeyType getType(DefaultMutableTreeNode node) {
        if (node.getLevel() == 1) {
            return KeyType.MAIN_KEY;
        }

        if (node.getLevel() != 3 && node.getLevel() != 5) {
            return KeyType.NONE;
        }
        return ((TextNode) ((DefaultMutableTreeNode) node.getParent()).getUserObject()).getType();
    }
}
