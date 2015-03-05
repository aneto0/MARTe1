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
 * Basic console implementation in linux
 */

#ifndef BASIC_CONSOLE_OS_H
#define BASIC_CONSOLE_OS_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define STDOUT 1
#define STDIN  0
/**
 * Number of rows that will be cleared when BasicConsoleOSClear is called
 */
#define BASIC_CONSOLE_LINUX_CLEAR_ROWS 40

/**
 * @see Console::Open
 */
bool BasicConsoleOSOpen(BasicConsole &con, int32 numberOfColumns,
                        int32 numberOfRows) {

    if (con.openingMode & PerformCharacterInput) {
        struct termio modifyConsoleModes;
        if (ioctl(fileno(stdin), TCGETA, &con.inputConsoleHandle) < 0) {
            return False;
        }
        modifyConsoleModes = con.inputConsoleHandle;
        modifyConsoleModes.c_lflag &= ~ICANON;
        modifyConsoleModes.c_cc[VMIN] = 1;
        modifyConsoleModes.c_cc[VTIME] = 0;
        ioctl(fileno(stdin), TCSETAW, &modifyConsoleModes);
    }
    fflush (stdin);
    return True;
}

/**
 * @see Console::Close
 */
bool BasicConsoleOSClose(BasicConsole &con) {
    if (con.openingMode & PerformCharacterInput) {
        ioctl(fileno(stdin), TCSETAW, &con.inputConsoleHandle);
    }
    return True;
}

/**
 * @see Console::Show
 */
bool BasicConsoleOSShow(BasicConsole &con) {
    return True;
}

/**
 * @see Console::Write
 */
bool BasicConsoleOSWrite(BasicConsole &con, const void* buffer, uint32 &size) {
    char* buffString = (char*) buffer;
    char nextRow = '\n';
    int32 n = 0;
    uint32 index = 0, sizeT = 0, start = 0;
    int32 columnLimit = con.numberOfColumns;
    while (1) {
        while (sizeT < columnLimit && index < size) {
            if (buffString[index] == '\0' || buffString[index] == '\n')
                break;
            index++;
            sizeT = index - start;
        }

        if (sizeT > 0)
            n += write(STDOUT, buffString + start, sizeT);

        if (index >= size)
            break;
        if (buffString[index] == '\0')
            break;

        write(STDOUT, &nextRow, 1);
        if (buffString[index] == '\n') {
            index++;
            n++;
        }
        start = index;
        sizeT = 0;
    }

    size = n;
    return n > 0;

}

/**
 * @see Console::Read
 */
bool BasicConsoleOSRead(BasicConsole &con, void* buffer, uint32 &size,
                        TimeoutType msecTimeout) {
    uint32 n = size;
    if (con.openingMode & PerformCharacterInput) {
        ((char *) buffer)[0] = (char) getchar();
        size = 1;
    }
    else {
        char *temp = NULL;
        while ((temp = fgets((char *) buffer, n, stdin)) == NULL)
            ;
        if ((n = strlen(temp)) > 0) {
            size = n;
        }
        temp[n] = '\0';
    }
    return (n > 0);
}

/**
 * Not implemented
 */
bool BasicConsoleOSSetTitleBar(BasicConsole &con, const char *title) {
    return False;
}

/**
 * Not implemented
 */
bool BasicConsoleOSSetWindowSize(BasicConsole &con, int numberOfColumns,
                                 int numberOfRows) {
    con.numberOfColumns = numberOfColumns;
    con.numberOfRows = numberOfRows;
    return True;
}

/**
 * Not implemented
 */
bool BasicConsoleOSGetWindowSize(BasicConsole &con, int &numberOfColumns,
                                 int &numberOfRows) {
    numberOfColumns = con.numberOfColumns;
    numberOfRows = con.numberOfRows;
    return True;
}

/**
 * Not implemented
 */
bool BasicConsoleOSSetSize(BasicConsole &con, int numberOfColumns,
                           int numberOfRows) {
    con.numberOfColumns = numberOfColumns;
    con.numberOfRows = numberOfRows;
    return True;
}

/**
 * Not implemented
 */
bool BasicConsoleOSGetSize(BasicConsole &con, int &numberOfColumns,
                           int &numberOfRows) {
    numberOfColumns = con.numberOfColumns;
    numberOfRows = con.numberOfRows;
    return True;
}

/**
 * Not implemented
 */
bool BasicConsoleOSSetCursorPosition(BasicConsole &con, int column, int row) {
    return False;
}

/**
 * Not implemented
 */
bool BasicConsoleOSGetCursorPosition(BasicConsole &con, int &column, int &row) {
    return False;
}

/**
 * Not implemented
 */
bool BasicConsoleOSSetColour(BasicConsole &con, Colours foreGroundColour,
                             Colours backGroundColour) {
    return False;
}

/**
 * @see BasicConsole::Clear
 */
bool BasicConsoleOSClear(BasicConsole &con) {
    for (int32 i = 0; i < BASIC_CONSOLE_LINUX_CLEAR_ROWS; i++) {
        write(STDOUT, "\n", 1);
    }
    return True;
}

/**
 * Not implemented
 */
bool BasicConsoleOSPlotChar(BasicConsole &con, char c, Colours foreGroundColour,
                            Colours backGroundColour, int column, int row) {
    return False;

}

#endif
