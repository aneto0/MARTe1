#!/usr/bin/python

import os, sys

def writeString2File(filename, string):
    if os.access(filename, os.W_OK):
        fd = open(filename, 'w')
        fd.write(string)
        try:
            fd.close()
            return True
        except IOError:
            print("Error: unable to close "+filename)
            return False
    else:
        print("Error: user doesn't have write permissions on "+filename)
        return False

def readStringFromFile(filename):
    if os.access(filename, os.R_OK):
        fd = open(filename, 'r')
        string = fd.read()
        fd.close()
        return string
    else:
        print("Error: user doesn't have read permissions on "+filename)
        return ""

START_PATH = "/proc/irq"
FILENAME   = "smp_affinity"
os.chdir(START_PATH)
fileList = os.listdir(".")

# Check arguments
numberOfArguments = len(sys.argv)-1
if numberOfArguments:
    if sys.argv[1] == "-h":
        print("Usage: "+sys.argv[0]+" [1st irq number] [2nd irq number] [...] [smp affinity mask]")
    elif numberOfArguments > 1:
        fileList = sys.argv[1:numberOfArguments]
    cpuMask = sys.argv[numberOfArguments]

# Do it
for filename in fileList:
    if os.path.isdir(filename) and filename[0].isdigit():
        if numberOfArguments:
            writeString2File(filename+"/"+FILENAME, cpuMask)
        print(filename+"/"+FILENAME+": cpu affinity mask = "+readStringFromFile(filename+"/"+FILENAME).rstrip('\n'))
