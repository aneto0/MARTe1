/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id: $
 *
 **/
/**
 * @file
 * Implements all the required functions to load and unload libraries
 */
#ifndef LOADABLE_LIBRARY_H
#define LOADABLE_LIBRARY_H

#include "GeneralDefinitions.h"

class LoadableLibrary;
extern "C" {
/**
 * @see LoadableLibrary::Open
 */
bool LoadableLibraryOpen(LoadableLibrary &ll, const char *dllName);

/**
 * @see LoadableLibrary::Close
 */
void LoadableLibraryClose(LoadableLibrary &ll);

/**
 * @see LoadableLibrary::Function.
 */
void *LoadableLibraryFunction(LoadableLibrary &ll, const char *name);
}

class LoadableLibrary {
public:
    HANDLE module;
private:
    bool LoadableLibraryOpen(LoadableLibrary &ll, const char *dllName);
    void LoadableLibraryClose(LoadableLibrary &ll);
    void *LoadableLibraryFunction(LoadableLibrary &ll, const char *name);

public:
    /** */
    LoadableLibrary() {
        module = NULL;
    }

    /** */
    ~LoadableLibrary() {
    }

    /**
     * The function uses OS functions to load a dynamic loadable library.
     * The VxWorks implementation however, does not provide a reliable method
     * to unload a module so this function is implemented in a different way.
     */
    bool Open(const char *dllName) {
        return LoadableLibraryOpen(*this, dllName);
    }

    /**
     * The function uses OS functions to unload a dynamic loadable library.
     */
    void Close() {
        return LoadableLibraryClose(*this);
    }

    /**
     * Returns the entry point of the module function specified by name.
     */
    void *Function(const char *name) {
        if (name == NULL) {
            return NULL;
        }
        return LoadableLibraryFunction(*this, name);
    }

    /** Returns the entry point of the module using the [] syntax.
     */
    void *operator[](const char *name) {
        return Function(name);
    }

};

#endif
