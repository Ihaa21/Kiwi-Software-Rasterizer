#if !defined(KIWI_MEMORY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct mem_arena
{
    mm Size;
    mm Used;
    u8* Mem;
};

struct temp_mem
{
    mem_arena* Arena;
    mm Used;
};

struct mem_double_arena
{
    mm Size;
    mm UsedTop;
    mm UsedBot;
    u8* Mem;
};

struct temp_double_mem
{
    mem_double_arena* Arena;
    mm UsedTop;
    mm UsedBot;
};

#define KIWI_MEMORY_H
#endif
