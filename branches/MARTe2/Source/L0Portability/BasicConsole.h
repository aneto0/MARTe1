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
 * The simplest human interface: a text terminal
 */
#ifndef BASIC_CONSOLE_H
#define BASIC_CONSOLE_H

#include "GeneralDefinitions.h"
#include "TimeoutType.h"

class BasicConsole;
/** flags determining the console operating modes */
enum ConsoleOpeningMode {
    /** Default mode */
    ConsoleDefault = 0,

    /** WIN32 only: operates on a display buffer different from the current one*/
    CreateNewBuffer = 1,

    /** Does not wait for \n\r */
    PerformCharacterInput = 2,

    /** CTRL-C does not create an exception */
    DisableControlBreak = 4,

    /** Enable paging */
    EnablePaging = 8
};

/** allows combining ConsoleOpeningMode data */
static inline ConsoleOpeningMode operator|(ConsoleOpeningMode a,
                                           ConsoleOpeningMode b) {
    return (ConsoleOpeningMode) ((int) a | (int) b);
}

/** allows combining ConsoleOpeningMode data */
static inline ConsoleOpeningMode operator&(ConsoleOpeningMode a,
                                           ConsoleOpeningMode b) {
    return (ConsoleOpeningMode) ((int) a & (int) b);
}

/** allows combining ConsoleOpeningMode data */
static inline ConsoleOpeningMode Not(ConsoleOpeningMode a) {
    return (ConsoleOpeningMode) (~(int) a);
}

extern "C" {
/** */
bool BasicConsoleOpen(BasicConsole &con, ConsoleOpeningMode openingMode,
                      int numberOfColumns, int numberOfRows,
                      TimeoutType msecTimeout = TTInfiniteWait);

/** */
bool BasicConsoleShow(BasicConsole &con);

/** */
bool BasicConsoleClose(BasicConsole &con);

/** */
bool BasicConsoleWrite(BasicConsole &con, const void* buffer, uint32 &size,
                       TimeoutType msecTimeout);

/** */
bool BasicConsoleRead(BasicConsole &con, void* buffer, uint32 &size,
                      TimeoutType msecTimeout);

/** */
bool BasicConsoleSetTitleBar(const char *title);

/** */
bool BasicConsoleSetSize(BasicConsole &con, int numberOfColumns,
                         int numberOfRows);

/** */
bool BasicConsoleGetSize(BasicConsole &con, int &numberOfColumns,
                         int &numberOfRows);

/** */
bool BasicConsoleSetWindowSize(BasicConsole &con, int numberOfColumns,
                               int numberOfRows);

/** */
bool BasicConsoleGetWindowSize(BasicConsole &con, int &numberOfColumns,
                               int &numberOfRows);

/** */
bool BasicConsoleSetCursorPosition(BasicConsole &con, int column, int row);

/** */
bool BasicConsoleGetCursorPosition(BasicConsole &con, int &column, int &row);

/** */
bool BasicBasicConsoleClear(BasicConsole &con);

/** */
bool BasicConsoleSetColour(BasicConsole &con, Colours foreGroundColour,
                           Colours backGroundColour);

/** */
bool BasicConsoleSwitch(BasicConsole &con, const char *name);

/** */
bool BasicConsolePlotChar(BasicConsole &con, char c, Colours foreGroundColour,
                          Colours backGroundColour, int column, int row);

}

/** Implements a tream interface to the console */
class BasicConsole {
    /** how many lines since last paging ?*/
    int32 lineCount;

    /** how long since last paging */
    int64 lastPagingTime;

    /** how long to wait when reading */
    TimeoutType msecTimeout;

    //These should be private and the friend functions in the OS implementation allowed to access this...
public:
    /** sets of flags describing the console status*/
    ConsoleOpeningMode openingMode;
    /** */
    struct termio inputConsoleHandle;

    /** */
    struct termio outputConsoleHandle;
private:

    friend bool BasicConsoleOpen(BasicConsole &con,
                                 ConsoleOpeningMode openingMode,
                                 int32 numberOfColumns, int32 numberOfRows,
                                 TimeoutType msecTimeout);

    friend bool BasicConsoleClose(BasicConsole &con);
    friend bool BasicConsoleShow(BasicConsole &con);
    friend bool BasicConsoleWrite(BasicConsole &con, const void* buffer,
                                  uint32 &size, TimeoutType msecTimeout);
    friend bool BasicConsoleRead(BasicConsole &con, void* buffer, uint32 &size,
                                 TimeoutType msecTimeout);
    friend bool BasicConsoleSetTitleBar(const char *title);
    friend bool BasicConsoleSetSize(BasicConsole &con, int32 numberOfColumns,
                                    int32 numberOfRows);
    friend bool BasicConsoleGetSize(BasicConsole &con, int32 &numberOfColumns,
                                    int32 &numberOfRows);
    friend bool BasicConsoleSetCursorPosition(BasicConsole &con, int32 column,
                                              int32 row);
    friend bool BasicConsoleSetWindowSize(BasicConsole &con,
                                          int32 numberOfColumns,
                                          int32 numberOfRows);
    friend bool BasicConsoleGetWindowSize(BasicConsole &con,
                                          int32 &numberOfColumns,
                                          int32 &numberOfRows);
    friend bool BasicConsoleGetCursorPosition(BasicConsole &con, int32 &column,
                                              int32 &row);
    friend bool BasicConsoleClear(BasicConsole &con);
    friend bool BasicConsoleSetColour(BasicConsole &con,
                                      Colours foreGroundColour,
                                      Colours backGroundColour);
    friend bool BasicConsoleSwitch(BasicConsole &con, const char *name);
    friend bool BasicConsolePlotChar(BasicConsole &con, char c,
                                     Colours foreGroundColour,
                                     Colours backGroundColour, int32 column,
                                     int32 row);

public:

    /** Creates a console stream with the desired parameters */
    BasicConsole(ConsoleOpeningMode openingMode = ConsoleDefault,
                 int32 numberOfColumns = -1, int32 numberOfRows = -1,
                 TimeoutType msecTimeout = TTInfiniteWait) {
        BasicConsoleOpen(*this, openingMode, numberOfColumns, numberOfRows,
                         msecTimeout);
    }

    /** destructor  */
    virtual ~BasicConsole() {
        BasicConsoleClose(*this);
    }

    /** Switches to display this console buffer. */
    inline bool Show() {
        return BasicConsoleShow(*this);
    }

protected:
    /** The actual Write */
    inline bool Write(const void* buffer, uint32 & size,
                      TimeoutType msecTimeout) {
        return BasicConsoleWrite(*this, buffer, size, msecTimeout);
    }

    /** The actual Read */
    inline bool Read(void* buffer, uint32 & size, TimeoutType msecTimeout) {
        return BasicConsoleRead(*this, buffer, size, msecTimeout);
    }

public:

    /** Set Title Bar text */
    inline bool SetTitleBar(const char *title) {
        return BasicConsoleSetTitleBar(title);
    }

    /** Sets the size of the buffer */
    inline bool SetSize(int32 numberOfColumns, int32 numberOfRows) {
        return BasicConsoleSetSize(*this, numberOfColumns, numberOfRows);
    }

    /** Retrieves the size of the buffer */
    inline bool GetSize(int32 & numberOfColumns, int32 & numberOfRows) {
        return BasicConsoleGetSize(*this, numberOfColumns, numberOfRows);
    }

    /** Sets the size of the window */
    inline bool SetWindowSize(int32 numberOfColumns, int32 numberOfRows) {
        return BasicConsoleSetWindowSize(*this, numberOfColumns, numberOfRows);
    }

    /** Returns the size of the window */
    inline bool GetWindowSize(int32 & numberOfColumns, int32 & numberOfRows) {
        return BasicConsoleGetWindowSize(*this, numberOfColumns, numberOfRows);
    }

    /** Sets the size position of the cursor */
    inline bool SetCursorPosition(int32 column, int32 row) {
        return BasicConsoleSetCursorPosition(*this, column, row);
    }

    /** Retrieves the size position of the cursor */
    inline bool GetCursorPosition(int32 & column, int32 & row) {
        return BasicConsoleGetCursorPosition(*this, column, row);
    }

    /** Sets the font fg and bg colours */
    inline bool SetColour(Colours foreGroundColour, Colours backGroundColour) {
        return BasicConsoleSetColour(*this, foreGroundColour, backGroundColour);
    }

    /** Clears content */
    inline bool Clear() {
        lineCount = 0;
        return BasicBasicConsoleClear(*this);
    }

    /** enable = False disables it */
    inline void SetPaging(bool enable) {
        lineCount = 0;
        if (enable) {
            openingMode = openingMode | EnablePaging;
        }
        else {
            openingMode = openingMode & Not(EnablePaging);
        }
    }

    /** write a single char on the console at a given position and a given colour set*/
    inline bool PlotChar(char c, Colours foreGroundColour,
                         Colours backGroundColour, int column, int row) {
        return BasicConsolePlotChar(*this, c, foreGroundColour,
                                    backGroundColour, column, row);
    }

};

#endif

