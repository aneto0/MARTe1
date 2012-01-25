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
#ifndef _RTAI_CONSOLE_API_H

#if defined __cplusplus
extern "C" {
#endif

/**
 * Call a RT function which lives in kernel space
 * This function expects to receive the following arguments:
 * remoteFunctionArguments[0] = functionPointer as provided by /proc/kallsyms
 * remoteFunctionArguments[1] = number of parameters for the function to be called
 * remoteFunctionArguments[2 + N] = parameter N address or value
 */
int CallRemoteFunction(int *remoteFunctionArguments);

/**
 * Allocates memory for a parameter in kernel space and copies the parameters
 * @parameter address: The address in user space
 * @parameter size: The number of bytes to copy
 * @returns: the address in kernel space (which you can use later for function calls)
 */
int CopyToKernel(int address, int size);

/**
 * Copies the contents of a parameter in kernel space back to user space
 * @parameter userSpaceAddress: The address in user space
 * @parameter kernelSpaceAddress: The address in kernel space
 * @parameter size: The number of bytes to copy
 * @returns: number of bytes that could not be copied. On success, this will be zero.
 */
int CopyFromKernel(int userSpaceAddress, int kernelSpaceAddress, int size);

/**
 * Frees previously allocated kernel memory
 * @parameter kernelAddress: The address in kernel space
 * @returns: always zero
 */
int FreeKernelMemory(int address);

/**
 * The pseudo file where the RTAI console is located.
 * The default location is: "/dev/RTAIConsole0"
 */
void SetConsoleLocation(const char *newConsoleLocation);

#if defined __cplusplus
}
#endif

#endif
