/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */
 
#include "kiwi_platform.h"
#include "kiwi.h"
#include "kiwi_assets.cpp"
#include "kiwi_render.cpp"

#define RENDER_DEPTH_MAP 0

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    u32 ScreenX = 512;
    u32 ScreenY = 512;
    game_state* GameState = (game_state*)GameMem->PermanentMem;
    if (!GameMem->IsInitialized)
    {
        GameState->Arena = InitArena((u8*)GameState + sizeof(game_state),
                                     GameMem->PermanentMemSize - sizeof(game_state));
        GameState->Assets = LoadAssets(GameMem, &GameState->Arena);

        GameState->RenderState = InitRenderState(512, 512, &GameState->Arena);
    }

    render_state* RenderState = &GameState->RenderState;
        
    // NOTE: Clear the screen
    for (u32 PixelId = 0; PixelId < ScreenX*ScreenY; ++PixelId)
    {
        RenderState->DepthMap[PixelId] = F32_MAX;
        RenderState->ColorMap[PixelId] = 0xFF;
    }

    f32 Texture[8*8] =
    {
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0,
        0, 1, 0, 1, 0, 1, 0, 1,
        1, 0, 1, 0, 1, 0, 1, 0,
    };

    vertex Triangle[3] =
    {
        { V4(1, 0, 0, 1), V2(1, 0) },
        { V4(0, 1, 0, 1), V2(0.5, 1) },
        { V4(-1, 0, 0, 1), V2(0, 0) },
    };

    // NOTE: Setup vertices for a box
    vertex Box[6*6] =
    {
        // NOTE: Front Face
        { {-0.5, -0.5, -0.5, 1}, {0, 0} },
        { {0.5, -0.5, -0.5, 1}, {0, 1} },
        { {0.5, 0.5, -0.5, 1}, {1, 1} },
        { {-0.5, -0.5, -0.5, 1}, {0, 0} },
        { {0.5, 0.5, -0.5, 1}, {1, 1} },
        { {-0.5, 0.5, -0.5, 1}, {0, 1} },

        // NOTE: Back Face
        { {-0.5, -0.5, 0.5, 1}, {0, 0} },
        { {0.5, -0.5, 0.5, 1}, {0, 1} },
        { {0.5, 0.5, 0.5, 1}, {1, 1} },
        { {-0.5, -0.5, 0.5, 1}, {0, 0} },
        { {0.5, 0.5, 0.5, 1}, {1, 1} },
        { {-0.5, 0.5, 0.5, 1}, {0, 1} },

        // NOTE: Left Face
        { {-0.5, -0.5, -0.5, 1}, {0, 0} },
        { {-0.5, 0.5, -0.5, 1}, {0, 1} },
        { {-0.5, 0.5, 0.5, 1}, {1, 1} },        
        { {-0.5, -0.5, -0.5, 1}, {0, 0} },
        { {-0.5, 0.5, 0.5, 1}, {1, 1} },
        { {-0.5, -0.5, 0.5, 1}, {0, 1} },

        // NOTE: Right Face
        { {0.5, -0.5, -0.5, 1}, {0, 0} },
        { {0.5, 0.5, -0.5, 1}, {0, 1} },
        { {0.5, 0.5, 0.5, 1}, {1, 1} },        
        { {0.5, -0.5, -0.5, 1}, {0, 0} },
        { {0.5, 0.5, 0.5, 1}, {1, 1} },
        { {0.5, -0.5, 0.5, 1}, {0, 1} },

        // NOTE: Top face
        { {-0.5, 0.5, -0.5, 1}, {0, 0} },
        { {0.5, 0.5, -0.5, 1}, {0, 1} },
        { {0.5, 0.5, 0.5, 1}, {1, 1} },        
        { {-0.5, 0.5, -0.5, 1}, {0, 0} },
        { {0.5, 0.5, 0.5, 1}, {1, 1} },
        { {-0.5, 0.5, 0.5, 1}, {0, 1} },

        // NOTE: Bottom face
        { {-0.5, -0.5, -0.5, 1}, {0, 0} },
        { {0.5, -0.5, -0.5, 1}, {0, 1} },
        { {0.5, -0.5, 0.5, 1}, {1, 1} },        
        { {-0.5, -0.5, -0.5, 1}, {0, 0} },
        { {0.5, -0.5, 0.5, 1}, {1, 1} },
        { {-0.5, -0.5, 0.5, 1}, {0, 1} },
    };
    
    local_global f32 SinT = 0.0f;
    SinT += 0.01f;
    if (SinT > 2.0f*Pi32)
    {
        SinT = 0.0f;
    }

#if 1
    for (i32 BoxY = -1; BoxY < 2; ++BoxY)
    {
        for (i32 BoxX = -1; BoxX < 2; ++BoxX)
        {
            m4 WorldMat = TranslationMat(2.0f*(f32)BoxX, 2.0f*(f32)BoxY, 4)*RotationMat(SinT, SinT, SinT)*ScaleMat(1, 1, 1);
            PushVertexArray(RenderState, Box, ArrayCount(Box), WorldMat, Texture);
        }
    }
#elif 0
    // NOTE: Render the sphere
    m4 WorldMat = TranslationMat(0, 0, 1)*RotationMat(0, 0, 0)*ScaleMat(0.2, 0.2, 0.2);
    asset_model* Model = GetModel(&GameState->Assets, Model_Sphere);
    PushModel(RenderState, Model, WorldMat);
#else
    // NOTE: Render the statue
    m4 WorldMat = TranslationMat(0, 0, 1)*RotationMat(0, 0, 0)*ScaleMat(0.0002, 0.0002, 0.0002);
    asset_model* Model = GetModel(&GameState->Assets, Model_Statue);
    PushModel(RenderState, Model, WorldMat);
#endif
    
    // NOTE: Push onto OpenGL
    ExecuteCommands(RenderState);
    
#if RENDER_DEPTH_MAP
    f32 Range = RenderState->Far - RenderState->Near;
    for (i32 PixelId = 0; PixelId < RenderState->ScreenX*RenderState->ScreenY; ++PixelId)
    {
        f32 Depth = (RenderState->DepthMap[PixelId] - RenderState->Near) / Range;
        RenderState->ColorMap[PixelId] = (((u32)(255.0f + 0.5f) << 24) |
                                          ((u32)(255.0f*Depth + 0.5f) << 16) |
                                          ((u32)(255.0f*Depth + 0.5f) << 8) |
                                          ((u32)(255.0f*Depth + 0.5f) << 0));
    }
#endif
    
    GameMem->PushTextureToScreen(ScreenX, ScreenY, RenderState->ColorMap);
}
