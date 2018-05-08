/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

global char* GlobalShaderHeaderCode = R"FOO(
#version 330 core

#define b32 bool
#define i32 int
#define u32 int unsigned
#define f32 float
#define v2 vec2
#define v3 vec3
#define v4 vec4
#define V2 vec2
#define V3 vec3
#define V4 vec4
#define m3 mat3
#define m4 mat4

)FOO";

global opengl OpenGL;

inline void GLCheckError()
{
#if COCO_DEBUG
    GLuint InvalidEnum = GL_INVALID_ENUM;
    GLuint InvalidValue = GL_INVALID_VALUE;
    GLuint InvalidOperation = GL_INVALID_OPERATION;
    GLuint InvalidFrameBufferOperation = GL_INVALID_FRAMEBUFFER_OPERATION;
    GLuint OutOfMemory = GL_OUT_OF_MEMORY;

    GLuint ErrorCode = glGetError();
    Assert(ErrorCode == GL_NO_ERROR);
#endif
}

inline b32 IsGLHandleValid(GLuint Handle)
{
    b32 Result = (Handle != -1);
    return Result;
}

internal GLuint OpenGLCreateProgram(char* HeaderCode, char* VertexCode, char* FragmentCode)
{
    GLuint Result = 0;
    
    GLint Temp = GL_FALSE;
    int InfoLogLength;
    
    GLuint VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* VertexShaderCode[] =
    {
        HeaderCode,
        VertexCode,
    };
    glShaderSource(VertexShaderId, ArrayCount(VertexShaderCode), VertexShaderCode, NULL);
    glCompileShader(VertexShaderId);
    glGetShaderiv(VertexShaderId, GL_COMPILE_STATUS, &Temp);
    glGetShaderiv(VertexShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        char InfoLog[1024];
        glGetShaderInfoLog(VertexShaderId, InfoLogLength, NULL, &InfoLog[0]);
        InvalidCodePath;
    }

    GLuint FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* FragShaderCode[] =
    {
        HeaderCode,
        FragmentCode,
    };
    glShaderSource(FragmentShaderId, ArrayCount(FragShaderCode), FragShaderCode, NULL);
    glCompileShader(FragmentShaderId);
    glGetShaderiv(FragmentShaderId, GL_COMPILE_STATUS, &Temp);
    glGetShaderiv(FragmentShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        char InfoLog[1024];
        glGetShaderInfoLog(FragmentShaderId, InfoLogLength, NULL, &InfoLog[0]);
        InvalidCodePath;
    }

    Result = glCreateProgram();
    glAttachShader(Result, VertexShaderId);
    glAttachShader(Result, FragmentShaderId);
    glLinkProgram(Result);

    glGetProgramiv(Result, GL_LINK_STATUS, &Temp);
    glGetProgramiv(Result, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        char InfoLog[1024];
        glGetProgramInfoLog(Result, InfoLogLength, NULL, &InfoLog[0]);
        InvalidCodePath;
    }

    glDetachShader(Result, VertexShaderId);
    glDetachShader(Result, FragmentShaderId);
    
    glDeleteShader(VertexShaderId);
    glDeleteShader(FragmentShaderId);

    return Result;
}

internal void CompileRenderFrame(render_frame_prog* RenderFrameProg)
{
    char* VertexCode = R"FOO(
layout(location = 0) in v3 VertPos;
layout(location = 1) in v2 VertUV;

smooth out v2 UV;

void main()
{
    gl_Position = V4(2.0*VertPos.xy, 1, 1);
    UV = VertUV;
}
)FOO";
    
    char* FragmentCode = R"FOO(
in v2 UV;
out v4 PixelColor;

uniform sampler2D TextureSampler;

void main() 
{
    PixelColor = texture(TextureSampler, UV).rgba;
}
)FOO";
    RenderFrameProg->ProgId = OpenGLCreateProgram(GlobalShaderHeaderCode, VertexCode, FragmentCode);
    RenderFrameProg->TexSamplerId = glGetUniformLocation(RenderFrameProg->ProgId, "TextureSampler");
} 

internal void BeginProgram(render_frame_prog Prog)
{
    glUseProgram(Prog.ProgId);

    u32 VertexSize = sizeof(v3) + sizeof(v2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexSize, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VertexSize, (void*)(sizeof(v3)));
                
    glUniform1i(Prog.TexSamplerId, 0);
}

internal void EndProgram(render_frame_prog Prog)
{
    glUseProgram(0);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

internal PUSH_TEXTURE_TO_SCREEN(GLPushTextureToScreen)
{
    glBindTexture(GL_TEXTURE_2D, OpenGL.FrameTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

internal void InitOpenGL(u32 ResWidth, u32 ResHeight)
{
    glEnable(GL_FRAMEBUFFER_SRGB);
    CompileRenderFrame(&OpenGL.RenderFrameProg);

    // NOTE: Create a box model
    f32 RectVerts[] =
    {
        -0.5, -0.5, 0, 0, 0,
        -0.5, 0.5, 0, 0, 1,
        0.5, 0.5, 0, 1, 1,
        -0.5, -0.5, 0, 0, 0,
        0.5, 0.5, 0, 1, 1,
        0.5, -0.5, 0, 1, 0,
    };

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glGenBuffers(1, &OpenGL.SquareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, OpenGL.SquareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectVerts), RectVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // NOTE: Init our rendered texture and fbo
    glGenTextures(1, &OpenGL.FrameTexture);
    glBindTexture(GL_TEXTURE_2D, OpenGL.FrameTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8_EXT, ResWidth, ResHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
