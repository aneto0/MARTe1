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
#include "BasicConsole.h"
#include "HighResolutionTimer.h"
#include INCLUDE_FILE_OPERATING_SYSTEM(OPERATING_SYSTEM,BasicConsoleOS.h)

bool BasicConsoleOpen(BasicConsole &con, ConsoleOpeningMode openingMode,
                      int32 numberOfColumns, int32 numberOfRows,
                      TimeoutType msecTimeout) {

    con.msecTimeout = msecTimeout;
    con.lineCount = 0;
    con.lastPagingTime = 0;
    con.openingMode = openingMode;

    return BasicConsoleOSOpen(con, numberOfColumns, numberOfRows);
}

bool BasicConsoleClose(BasicConsole &con) {
    return BasicConsoleOSClose(con);
}

bool BasicConsoleShow(BasicConsole &con) {
    return BasicConsoleOSShow(con);
}

bool BasicConsoleWrite(BasicConsole &con, const void* buffer, uint32 &size,
                       TimeoutType msecTimeout) {

    int32 numberOfColumns;
    int32 numberOfRows;
    if ((con.openingMode & EnablePaging)
            && (BasicConsoleGetWindowSize(con, numberOfColumns, numberOfRows))) {

        int64 t0 = con.lastPagingTime;
        int64 t1 = HighResolutionTimer::Counter();

        int64 dT = t1 - t0;
        double dt = dT * HighResolutionTimer::Period();
        if (dt > 0.05) {
            con.lineCount = 0;
            con.lastPagingTime = t1;
        }

        char *p = (char *) buffer;
        uint32 index = 0;
        int start = 0;
        while (index < size) {
            while ((con.lineCount < (numberOfRows - 1)) && (index < size)) {
                if (p[index] == '\n')
                    con.lineCount++;
                index++;
            }
            size = index - start;
            BasicConsoleOSWrite(con, p + start, size);
            if (con.lineCount >= (numberOfRows - 1)) {
                start = index;
                con.lastPagingTime = t1;
                con.lineCount = 0;
                const char *message = "[PAGING] ENTER TO CONTINUE\015";
                size = strlen(message);
                BasicConsoleOSWrite(con, message, size);
                char buffer[32];
                uint32 size = 1;
                BasicConsoleRead(con, buffer, size, msecTimeout);
            }
        }
        return True;
    }
    else {
        return BasicConsoleOSWrite(con, buffer, size);
    }
}

bool BasicConsoleRead(BasicConsole &con, void* buffer, uint32 &size,
                      TimeoutType msecTimeout) {
    return BasicConsoleOSRead(con, buffer, size, msecTimeout);
}

bool BasicConsoleSetTitleBar(BasicConsole &con, const char *title) {
    return BasicConsoleOSSetTitleBar(con, title);
}

bool BasicConsoleSetWindowSize(BasicConsole &con, int32 numberOfColumns,
                               int32 numberOfRows) {
    return BasicConsoleOSSetWindowSize(con, numberOfColumns, numberOfRows);
}

bool BasicConsoleGetWindowSize(BasicConsole &con, int32 &numberOfColumns,
                               int32 &numberOfRows) {
    return BasicConsoleOSGetWindowSize(con, numberOfColumns, numberOfRows);
}

bool BasicConsoleConsoleSetSize(BasicConsole &con, int32 numberOfColumns, int32 numberOfRows) {
    return BasicConsoleOSSetSize(con, numberOfColumns, numberOfRows);
}

bool BasicConsoleGetSize(BasicConsole &con, int32 &numberOfColumns,
                    int &numberOfRows) {
    return BasicConsoleOSGetSize(con, numberOfColumns, numberOfRows);
}

bool BasicConsoleSetCursorPosition(BasicConsole &con, int32 column, int32 row) {
    return BasicConsoleOSSetCursorPosition(con, column, row);
}

bool BasicConsoleGetCursorPosition(BasicConsole &con, int32 &column, int32 &row) {
    return BasicConsoleOSGetCursorPosition(con, column, row);
}

bool BasicConsoleSetColour(BasicConsole &con, Colours foreGroundColour,
                      Colours backGroundColour) {
    return BasicConsoleOSSetColour(con, foreGroundColour, backGroundColour);
}

bool BasicConsoleClear(BasicConsole &con) {
    return BasicConsoleOSClear(con);
}

bool BasicConsolePlotChar(BasicConsole &con, char c, Colours foreGroundColour,
                     Colours backGroundColour, int32 column, int32 row) {
    return BasicConsoleOSPlotChar(con, c, foreGroundColour, backGroundColour, column, row);
}

