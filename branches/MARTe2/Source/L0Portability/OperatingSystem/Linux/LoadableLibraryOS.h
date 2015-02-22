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
 * Loadable library implementation in Linux
 */
#ifndef LOADABLE_LIBRARY_OS_H
#define LOADABLE_LIBRARY_OS_H

#include <dlfcn.h>

/**
 * @see LoadableLibrary::Close
 */
void LoadableLibraryOSClose(LoadableLibrary &ll) {
    if (ll.module != 0) {
        dlclose (ll.module);
    }
}

/**
 * @see LoadableLibrary::Open
 */
bool LoadableLibraryOSOpen(LoadableLibrary &ll, const char *dllName) {
    if (ll.module != 0) {
        LoadableLibraryOSClose(ll);
    }

    ll.module = dlopen(dllName, RTLD_NOW | RTLD_GLOBAL);
    if (ll.module == NULL) {
        return False;
    }
    return True;
}

/**
 * @see LoadableLibrary::Function.
 */
void *LoadableLibraryOSFunction(LoadableLibrary &ll, const char *name) {
    if ((ll.module == NULL) || (name == NULL)) {
        return NULL;
    }
    return dlsym(ll.module, name);
}

#endif
