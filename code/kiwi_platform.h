#if !defined(KIWI_PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <stdio.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t mm;
typedef uintptr_t umm;

typedef int32_t b32;

#define internal static
#define global static
#define local_global static

#define snprintf _snprintf_s
#define Assert(Expression) if (!(Expression)) {*(int*)0 = 0;}
#define InvalidCodePath Assert(!"Invalid Code Path")
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define KiloBytes(Val) ((Val)*1024LL)
#define MegaBytes(Val) (KiloBytes(Val)*1024LL)
#define GigaBytes(Val) (MegaBytes(Val)*1024LL)
#define TeraBytes(Val) (GigaBytes(Val)*1024LL)

struct game_memory;
struct platform_api;
extern struct platform_api PlatformApi;

//
//
//

#include <intrin.h>
#include "kiwi_math.h"
#include "kiwi_math.cpp"
#include "kiwi_memory.h"
#include "kiwi_memory.cpp"
#include "kiwi_assets.h"
#include "kiwi_render.h"

typedef struct game_button
{
    u16 Down;
    u16 Pressed;
} game_button;

typedef struct game_input
{
    b32 MouseDown;
    v2 MouseP;
    
    union
    {
        struct
        {
            b32 MoveLeftDown;
            b32 MoveRightDown;
            b32 MoveUpDown;
            b32 MoveDownDown;
        };

        b32 ButtonsPressed[4];
    };
} game_input;

struct game_render_settings
{
    // NOTE: This stores the properties for rendering that the OS defines
    u32 ResWidth;
    u32 ResHeight;
    f32 FrameTime;
};

struct game_render_commands
{
    // NOTE: This stores the data the game populates for rendering. This is taken by GL later to
    // send to the GPU
    mem_double_arena Arena;
    u32 WhiteTextureId;

    game_render_settings Settings;
    f32 AspectRatio;

    m4 CameraMat;
    m4 ProjMat;
};

#define PUSH_TEXTURE_TO_SCREEN(name) void name(u32 Width, u32 Height, void* Pixels)
typedef PUSH_TEXTURE_TO_SCREEN(push_texture_to_screen);

typedef struct game_memory
{
    b32 IsInitialized;
    mm PermanentMemSize;
    void* PermanentMem;

    mm AssetMemSize;
    u8* AssetMem;

    push_texture_to_screen* PushTextureToScreen;
} game_memory;

//
//
//

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory* GameMem, game_input* Input, \
                                               game_render_settings RenderSettings)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

//
//
//

#define KIWI_PLATFORM_H
#endif
