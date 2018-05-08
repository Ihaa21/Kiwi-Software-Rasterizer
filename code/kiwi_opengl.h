#if !defined(KIWI_OPENGL_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

struct render_frame_prog
{
    GLuint ProgId;

    GLuint TexSamplerId;
};

struct opengl
{
    GLuint FrameTexture;
    GLuint SquareVBO;
    render_frame_prog RenderFrameProg;
};

#define KIWI_OPENGL_H
#endif
