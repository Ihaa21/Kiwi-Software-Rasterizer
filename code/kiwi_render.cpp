/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

inline render_state InitRenderState(u32 ScreenX, u32 ScreenY, mem_arena* Arena)
{
    render_state Result = {};
    
    Result.ScreenX = 512;
    Result.ScreenY = 512;
    Result.DepthMap = PushArray(Arena, f32, ScreenX*ScreenY);
    Result.ColorMap = PushArray(Arena, u32, ScreenX*ScreenY);
    
    u64 MemSize = MegaBytes(10);
    void* Mem = PushSize(Arena, MemSize);
    Result.Arena = InitArena(Mem, MemSize);

    f32 AspectRatio = (f32)ScreenX / (f32)ScreenY;
    f32 Fov = Pi32/2.0f;
    Result.Near = 0.001f;
    Result.Far = 100.0f;

    Result.ProjMat = {};
    Result.ProjMat.v[0].x = 1.0f / (AspectRatio*Tan(Fov / 2.0f));
    Result.ProjMat.v[1].y = 1.0f / Tan(Fov / 2.0f);
    Result.ProjMat.v[2].z = -(Result.Far + Result.Near) / (Result.Near - Result.Far);
    Result.ProjMat.v[2].w = 1.0f;
    Result.ProjMat.v[3].z = 2.0f*Result.Far*Result.Near/(Result.Near - Result.Far);

    return Result;
}

inline void PushVertexArray(render_state* RenderState, vertex* VertexArray, u32 NumVertices,
                            m4 WorldMat, f32* Texture)
{
    u32* Cmd = PushStruct(&RenderState->Arena, u32);
    *Cmd = RenderCmdType_DrawVertexArray;

    render_vertices_cmd* RenderVertices = PushStruct(&RenderState->Arena, render_vertices_cmd);
    RenderVertices->NumVertices = NumVertices;
    RenderVertices->Texture = Texture;
    RenderVertices->WorldMat = WorldMat;

    vertex* RenderVertexArray = PushArray(&RenderState->Arena, vertex, NumVertices);
    Copy(VertexArray, RenderVertexArray, sizeof(vertex)*NumVertices);
}

inline void PushModel(render_state* RenderState, asset_model* Model, m4 WorldMat)
{
    u32* Cmd = PushStruct(&RenderState->Arena, u32);
    *Cmd = RenderCmdType_DrawVertexArray;

    render_vertices_cmd* RenderVertices = PushStruct(&RenderState->Arena, render_vertices_cmd);
    RenderVertices->Texture = 0;
    RenderVertices->WorldMat = WorldMat;

    for (u32 MeshId = 0; MeshId < Model->NumMeshes; ++MeshId)
    {
        asset_mesh* Mesh = Model->Meshes + MeshId;
        vertex* RenderVertexArray = PushArray(&RenderState->Arena, vertex, Mesh->NumVertices);
        Copy(Mesh->Vertices, RenderVertexArray, sizeof(vertex)*Mesh->NumVertices);

        RenderVertices->NumVertices += Mesh->NumVertices;
    }
}

inline v2 ClipToScreenCoords(render_state* RenderState, v2 Pos)
{
    v2 Result = {};
    Result = 0.5f*(Pos + V2(1, 1));
    Result.x *= (f32)RenderState->ScreenX;
    Result.y *= (f32)RenderState->ScreenY;

    return Result;
}

inline f32 CalcBaryCentric(v2 P1, v2 P2, v2 P)
{
    f32 Result = ((P2.x - P1.x)*(P.y - P1.y) - (P2.y - P1.y)*(P.x - P1.x));
    return Result;
}

inline void Swap(v2* A, v2* B)
{
    v2 Temp = *A;
    *A = *B;
    *B = Temp;
}

inline void BubbleSort(f32* SortKeys, u32* SortVals, u32 NumElements)
{
    while (true)
    {
        b32 Swapped = false;
        for (u32 Index = 1; Index < NumElements; ++Index)
        {
            if (SortKeys[Index-1] < SortKeys[Index])
            {
                {
                    f32 Temp = SortKeys[Index-1];
                    SortKeys[Index-1] = SortKeys[Index];
                    SortKeys[Index] = Temp;
                }

                {
                    u32 Temp = SortVals[Index-1];
                    SortVals[Index-1] = SortVals[Index];
                    SortVals[Index] = Temp;
                }
                
                Swapped = true;
            }
        }

        if (!Swapped)
        {
            break;
        }
    }
}

inline void ExecuteCommands(render_state* RenderState)
{
    u64 CurrByte = 0;
    while (CurrByte < RenderState->Arena.Used)
    {
        u32* CmdType = (u32*)((u8*)RenderState->Arena.Mem + CurrByte);
        CurrByte += sizeof(u32);

        void* RenderCmd = (u8*)RenderState->Arena.Mem + CurrByte;

        switch (*CmdType)
        {
            case RenderCmdType_DrawVertexArray:
            {
                render_vertices_cmd* VerticesCmd = (render_vertices_cmd*)RenderCmd;
                CurrByte += sizeof(render_vertices_cmd);

                Assert(IsDivisible(VerticesCmd->NumVertices, 3));

                vertex* VertexArray = (vertex*)((u8*)RenderState->Arena.Mem + CurrByte);
                m4 WVPTransform = RenderState->ProjMat*VerticesCmd->WorldMat;

                // NOTE: We transform the vertices in the vertex buffer to pixel space
                for (u32 TriangleId = 0; TriangleId < (VerticesCmd->NumVertices/3); ++TriangleId)
                {
                    vertex* Vertex = VertexArray + TriangleId*3;
                    
                    Vertex[0].Pos = Vertex[0].Pos*WVPTransform;
                    Vertex[1].Pos = Vertex[1].Pos*WVPTransform;
                    Vertex[2].Pos = Vertex[2].Pos*WVPTransform;

                    // NOTE: Perspective divide
                    Vertex[0].Pos.xy /= Vertex[0].Pos.w;
                    Vertex[1].Pos.xy /= Vertex[1].Pos.w;
                    Vertex[2].Pos.xy /= Vertex[2].Pos.w;

                    // NOTE: Apply z to interpolated attributes
                    Vertex[0].Pos.z = 1.0f / Vertex[0].Pos.z;
                    Vertex[1].Pos.z = 1.0f / Vertex[1].Pos.z;
                    Vertex[2].Pos.z = 1.0f / Vertex[2].Pos.z;
                    Vertex[0].UV *= Vertex[0].Pos.z;
                    Vertex[1].UV *= Vertex[1].Pos.z;
                    Vertex[2].UV *= Vertex[2].Pos.z;
        
                    // NOTE: Convert to screen coordinates
                    Vertex[0].Pos.xy = ClipToScreenCoords(RenderState, Vertex[0].Pos.xy);
                    Vertex[1].Pos.xy = ClipToScreenCoords(RenderState, Vertex[1].Pos.xy);
                    Vertex[2].Pos.xy = ClipToScreenCoords(RenderState, Vertex[2].Pos.xy);
        
                    // NOTE: Sort vertices for CCW winding
                    v2 TriCenter = (Vertex[0].Pos.xy + Vertex[1].Pos.xy + Vertex[2].Pos.xy) / 3.0f;
                    v2 Delta1 = Vertex[0].Pos.xy - TriCenter;
                    v2 Delta2 = Vertex[1].Pos.xy - TriCenter;
                    v2 Delta3 = Vertex[2].Pos.xy - TriCenter;

                    f32 SortKeys[3] = {ArcTan(Delta1.y, Delta1.x),
                                       ArcTan(Delta2.y, Delta2.x),
                                       ArcTan(Delta3.y, Delta3.x)};
                    u32 SortVals[3] = {0, 1, 2};
                    BubbleSort(SortKeys, SortVals, 3);

                    vertex TempVerts[3] = { Vertex[0], Vertex[1], Vertex[2] };
                    Vertex[0] = TempVerts[SortVals[0]];
                    Vertex[1] = TempVerts[SortVals[1]];
                    Vertex[2] = TempVerts[SortVals[2]];
                }

                for (u32 TriangleId = 0; TriangleId < (VerticesCmd->NumVertices/3); ++TriangleId)
                {
                    vertex* Vertex = VertexArray + TriangleId*3;
                    
                    // NOTE: If triangle near plane, skip it
                    if (Vertex[0].Pos.z <= RenderState->Near &&
                        Vertex[1].Pos.z <= RenderState->Near &&
                        Vertex[2].Pos.z <= RenderState->Near)
                    {
                        continue;
                    }
                    
                    // NOTE: Build bounding box around triangle on screen
                    i32 MinX = RoundToI32(Min(Vertex[0].Pos.x, Min(Vertex[1].Pos.x, Vertex[2].Pos.x)));
                    i32 MaxX = RoundToI32(Max(Vertex[0].Pos.x, Max(Vertex[1].Pos.x, Vertex[2].Pos.x)));
                    i32 MinY = RoundToI32(Min(Vertex[0].Pos.y, Min(Vertex[1].Pos.y, Vertex[2].Pos.y)));
                    i32 MaxY = RoundToI32(Max(Vertex[0].Pos.y, Max(Vertex[1].Pos.y, Vertex[2].Pos.y)));

                    // NOTE: Clip triangle rasterization to screen bounds
                    MinX = Min(Max(0, MinX), RenderState->ScreenX-1);
                    MaxX = Max(0, Min(RenderState->ScreenX-1, MaxX));
                    MinY = Min(Max(0, MinY), RenderState->ScreenY-1);
                    MaxY = Max(0, Min(RenderState->ScreenY-1, MaxY));

                    v2 Edge1 = Vertex[2].Pos.xy - Vertex[1].Pos.xy;
                    v2 Edge2 = Vertex[0].Pos.xy - Vertex[2].Pos.xy;
                    v2 Edge3 = Vertex[1].Pos.xy - Vertex[0].Pos.xy;

                    f32 DeltaX1 = -Edge1.y;
                    f32 DeltaX2 = -Edge2.y;
                    f32 DeltaX3 = -Edge3.y;

                    f32 DeltaY1 = Edge1.x;
                    f32 DeltaY2 = Edge2.x;
                    f32 DeltaY3 = Edge3.x;

                    f32 CenterMinX = (f32)MinX + 0.5f;
                    f32 CenterMaxX = (f32)MaxX + 0.5f;
                    f32 CenterMinY = (f32)MinY + 0.5f;
                    f32 CenterMaxY = (f32)MaxY + 0.5f;
                    
                    f32 W1Row = DeltaY1*(CenterMinY - Vertex[1].Pos.y) + DeltaX1*(CenterMinX - Vertex[1].Pos.x);
                    f32 W2Row = DeltaY2*(CenterMinY - Vertex[2].Pos.y) + DeltaX2*(CenterMinX - Vertex[2].Pos.x);
                    f32 W3Row = DeltaY3*(CenterMinY - Vertex[0].Pos.y) + DeltaX3*(CenterMinX - Vertex[0].Pos.x);
    
                    f32 Area = CalcBaryCentric(Vertex[0].Pos.xy, Vertex[1].Pos.xy, Vertex[2].Pos.xy);
                    u32 NumPixels = 0;
                    for (f32 Y = CenterMinY; Y <= CenterMaxY; Y += 1.0f)
                    {
                        ++NumPixels;
                        f32 W1 = W1Row;
                        f32 W2 = W2Row;
                        f32 W3 = W3Row;
        
                        for (f32 X = CenterMinX; X <= CenterMaxX; X += 1.0f)
                        {
                            if (W1 >= 0.0f && W2 >= 0.0f && W3 >= 0.0f)
                            {
                                // NOTE: Top edge is one that is horizontal and goes to the left (CCW)
                                // NOTE: Left edge is one that goes down (CCW)
                                bool Overlaps = true;
                                Overlaps &= (W1 == 0 ? ((Edge1.y == 0.0f && Edge1.x > 0.0f) || Edge1.y > 0.0f) : (W1 > 0));
                                Overlaps &= (W2 == 0 ? ((Edge2.y == 0.0f && Edge2.x > 0.0f) || Edge2.y > 0.0f) : (W2 > 0));
                                Overlaps &= (W3 == 0 ? ((Edge3.y == 0.0f && Edge3.x > 0.0f) || Edge3.y > 0.0f) : (W3 > 0));

                                if (!Overlaps)
                                {
                                    continue;
                                }
                                
                                f32 TempW1 = W1 / Area;
                                f32 TempW2 = W2 / Area;
                                f32 TempW3 = W3 / Area;

                                u32 ScreenIndex = (u32)Y*RenderState->ScreenX + (u32)X;
                                f32 PixelZ = 1.0f / (TempW1*Vertex[0].Pos.z + TempW2*Vertex[1].Pos.z + TempW3*Vertex[2].Pos.z);
                                if (PixelZ < RenderState->DepthMap[ScreenIndex])
                                {
                                    RenderState->DepthMap[ScreenIndex] = PixelZ;

                                    f32 Texel = 1.0f;
                                    if (VerticesCmd->Texture)
                                    {
                                        // NOTE: Interpolate texture coords
                                        v2 PixelUV = TempW1*Vertex[0].UV + TempW2*Vertex[1].UV + TempW3*Vertex[2].UV;
                                        PixelUV *= 8.0f*PixelZ;
                                        Texel = VerticesCmd->Texture[(u32)PixelUV.y*8 + (u32)PixelUV.x];
                                    }
                                    
                                    RenderState->ColorMap[ScreenIndex] = (((u32)(255.0f + 0.5f) << 24) |
                                                                          ((u32)(255.0f*Texel + 0.5f) << 16) |
                                                                          ((u32)(255.0f*Texel + 0.5f) << 8) |
                                                                          ((u32)(255.0f*Texel + 0.5f) << 0));
                                    
                                }
                            }
            
                            W1 += DeltaX1;
                            W2 += DeltaX2;
                            W3 += DeltaX3;
                        }

                        W1Row += DeltaY1;
                        W2Row += DeltaY2;
                        W3Row += DeltaY3;
                    }

                    if (NumPixels == 0)
                    {
                        int i = 0;
                    }

                }
            } break;
        }
    }
}
