#if !defined(KIWI_RENDER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

enum RenderCmdType
{
    RenderCmdType_None,
    
    RenderCmdType_DrawVertexArray,
};

struct vertex
{
    v4 Pos;
    v2 UV;
};

struct render_vertices_cmd
{
    u32 NumVertices;
    u32 TextureX;
    u32 TextureY;
    f32* Texture;
    m4 WorldMat;
};

struct render_state
{
    i32 ScreenX;
    i32 ScreenY;
    f32* DepthMap;
    u32* ColorMap;

    mem_arena Arena;
    f32 Near;
    f32 Far;
    m4 ProjMat;
    m4 CameraMat;
};

#define KIWI_RENDER_H
#endif
