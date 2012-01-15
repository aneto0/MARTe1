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
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>

// Time between timer calls (1 millisecond)
#define INT_LENGTH 1 * HZ / 1000

// Name of the device created
#define DEVNAME "LinuxTimer"

// Flag turned at 1 when the timer runs
static int TimerRun;

// Major and minor device number, char device structure
static int LTMajor;
static int LTMinor;
static struct cdev LTCharDev;

// List of timer set
struct timer_list MyTimer;

// Static init of the wait queue structure
DECLARE_WAIT_QUEUE_HEAD(LTQueue);

// File operations on the char device. 
// Most are simply NOP (we need only read)
int LTFnop(struct inode *inode, struct file *filp){
	return 0;
}
int LTFIoctl(struct inode *inode, struct file *filp, unsigned int fun, unsigned long argp){
	return 0;
}
static ssize_t LTFWrite(struct file *filp, const char *buf, size_t count, loff_t *f_pos){
	return 0;
}

// Blocking read: returns only on the event of a timer (TimerRun==1)
ssize_t LTFRead(struct file *filp, char *buf, size_t count, loff_t *f_pos){
	// Wait until TimerRun==1
	wait_event_interruptible(LTQueue, TimerRun == 1);
	TimerRun=0;
	buf[0]=1;
	return 1;
}

// Structure pointing to the file ops functions
static const struct file_operations LTFOps = {
	read: LTFRead,
	write: LTFWrite,
	ioctl: LTFIoctl,
	open: LTFnop,
	release: LTFnop
};

// Function callback for the timer
void Hook(unsigned long data){
//	struct timespec temp;
//	temp=current_kernel_time();
//	printk("I have been called at %d\n",(int)temp.tv_sec);
	TimerRun = 1;
	
	// Timer has been executed, wake up if needed LTFRead
	wake_up_interruptible(&LTQueue);
	
	// Re-insert timer in the MyTimer structure
	mod_timer(&MyTimer,jiffies+INT_LENGTH);
}

// Module initialization
static int __init init_LT(void){
	int result = 0;
	dev_t dev_id;
	TimerRun = 0;

	// Char dev init - dynamic dev number allocation
	// remember to use ./loadTimer.cmd to autocreate the device in /dev
	result = alloc_chrdev_region(&dev_id,0,1,DEVNAME);
	LTMajor = MAJOR(dev_id);
	LTMinor = MINOR(dev_id);
	if (result < 0) {
		printk("%s: alloc_chrdev_region failed\n", DEVNAME);
		return 1;
	}
	cdev_init(&LTCharDev, &LTFOps);
	LTCharDev.owner = THIS_MODULE;
	LTCharDev.ops = &LTFOps;
	result = cdev_add(&LTCharDev,dev_id,1);
	if (result < 0) {
		printk("%s: cdev_add failed\n", DEVNAME);
		return 1;
	}
	

	// Timer init
	init_timer(&MyTimer);
	MyTimer.expires = jiffies + msecs_to_jiffies(1000);//INT_LENGTH;
	// no need for any special data in the callback function
	MyTimer.data = 0;
	// Callback function
	MyTimer.function = Hook;
	add_timer(&MyTimer);

	return 0;
}

// Module cleanup
static void cleanup_LT(void){
	// CharDev deletion
	dev_t dev_id;
	dev_id=MKDEV(LTMajor,LTMinor);
	unregister_chrdev_region(dev_id,1);
	cdev_del(&LTCharDev);	

	// Timer deletion
	del_timer_sync(&MyTimer);
	return;
}

module_init(init_LT);
module_exit(cleanup_LT);
