#if !defined(WIN32_KIWI_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct win32_game_code
{
    char* SourceDLLPath;
    char* TempDLLPath;
    char* LockFilePath;
    
    HMODULE GameCodeDLL;
    FILETIME LastDLLFileTime;

    // IMPORTANT: Either of the callbacks can be null
    // You must check before calling
    game_update_and_render* UpdateAndRender;
};

struct win32_state
{
    b32 GameIsRunning;
    i64 TimerFrequency;

    HDC DeviceContext;
};

#define WIN32_KIWI_H
#endif
