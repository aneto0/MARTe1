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

#ifndef PCIEADC_PROC_H_
#define PCIEADC_PROC_H_
#include <linux/version.h>
#include <linux/seq_file.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
int pcieAdc_procinfo(char *buf, char **start, off_t fpos, int length, int *eof, void *data);
int pcieAdc_proccmd(struct file *file, const char __user *buffer, unsigned long count, void *data);
#else
int pcieAdc_show(struct seq_file *fl, void *v);
int pcieAdc_open(struct inode *inode, struct file *file);
int pcieAdc_proccmd(struct file * flp, const char __user *usr, size_t count, loff_t * fost);
#endif
void register_proc(void);
void unregister_proc(void);

#endif /*PCIEADC_PROC_H_*/
