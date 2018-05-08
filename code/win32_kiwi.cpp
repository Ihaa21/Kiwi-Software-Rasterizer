#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <windows.h>
#include <DbgHelp.h>
#include <dsound.h>

#include "kiwi_platform.h"
#include "kiwi_string.h"
#include "win32_kiwi.h"

//
//
//

global HGLRC GlobalGLContext;
global win32_state GlobalState;

#include <gl/gl.h>
#include <gl/wglext.h>
#include "win32_kiwi_opengl.cpp"
#include "kiwi_opengl.h"
#include "kiwi_opengl.cpp"

//
//
//

inline void Win32ReadAssetFile(game_memory* Memory)
{
    HANDLE FileHandle = CreateFileA("kiwi.assets", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        InvalidCodePath;
    }
    
    LARGE_INTEGER FileSize;
    if(!GetFileSizeEx(FileHandle, &FileSize))
    {
        InvalidCodePath;
    }
    
    u32 FileSize32 = SafeTruncateU64(FileSize.QuadPart);        
    void* Data = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!Data)
    {
        InvalidCodePath;
    }

    DWORD BytesRead;
    if(!(ReadFile(FileHandle, Data, FileSize32, &BytesRead, 0) &&
         (FileSize32 == BytesRead)))
    {
        InvalidCodePath;
    }
            
    // NOTE(casey): File read successfully
    Memory->AssetMemSize = FileSize32;
    Memory->AssetMem = (u8*)Data;
    CloseHandle(FileHandle);
}

//
//
//

inline LARGE_INTEGER Win32GetClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);

    return Result;
}

inline f32 Win32GetSecondsBetween(LARGE_INTEGER End, LARGE_INTEGER Start)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)GlobalState.TimerFrequency);
    return Result;
}

inline FILETIME Win32GetLastFileTime(char* FileName)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA FileData;
    if (GetFileAttributesEx(FileName, GetFileExInfoStandard, &FileData))
    {
        LastWriteTime = FileData.ftLastWriteTime;
    }

    return LastWriteTime;
}

internal LRESULT CALLBACK Win32MainWindowCallBack(HWND Window, UINT Message, WPARAM WParam,
                                                  LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            GlobalState.GameIsRunning = false;
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

inline DWORD Win32GetNearestFrameByte(DWORD SoundBytesPerFrame, DWORD CurrentSoundByte)
{
    // TODO: Implement a floor function here for NearestFrameCount
    DWORD NearestFrameCount = (DWORD)((CurrentSoundByte + SoundBytesPerFrame) / SoundBytesPerFrame);
    DWORD Result = NearestFrameCount*SoundBytesPerFrame;
    return Result;
}

inline void Win32LoadGameCode(win32_game_code* GameCode)
{
    // NOTE: Unload the game code
    if (GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }

    GameCode->UpdateAndRender = 0;
    
    // NOTE: Load the game code
    b32 IsValid = false;
    for (u32 LoadTryIndex = 0; LoadTryIndex < 100 && !IsValid; ++LoadTryIndex)
    {
        WIN32_FILE_ATTRIBUTE_DATA Ignored;
    
        // NOTE: We check if the lock file exists. The lock file is there so that we
        // don't load the dll before the pdb
        if (!GetFileAttributesEx(GameCode->LockFilePath, GetFileExInfoStandard, &Ignored))
        {
            GameCode->LastDLLFileTime = Win32GetLastFileTime(GameCode->SourceDLLPath);
            CopyFileA(GameCode->SourceDLLPath, GameCode->TempDLLPath, FALSE);
            GameCode->GameCodeDLL = LoadLibraryA(GameCode->TempDLLPath);

            if (!GameCode->GameCodeDLL)
            {
                InvalidCodePath;
            }

            // NOTE: Load in the functions from our DLL
            GameCode->UpdateAndRender = (game_update_and_render*)GetProcAddress(GameCode->GameCodeDLL, "GameUpdateAndRender");
                        
            IsValid = true;
        }
                
        Sleep(100);
    }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    GlobalState.GameIsRunning = true;
        
    // NOTE: Init window and display
    game_render_settings Settings = {};
    Settings.ResWidth = 512;
    Settings.ResHeight = 512;
    Settings.FrameTime = 1.0f / 60.0f;
    HWND Window;
    {
        WNDCLASSA WindowClass = {};
        WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        WindowClass.lpfnWndProc = Win32MainWindowCallBack;
        WindowClass.hInstance = hInstance;
        WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
        WindowClass.lpszClassName = "KiwiClass";

        if (!RegisterClassA(&WindowClass))
        {
            // TODO: Display error message
            InvalidCodePath;
        }
        
        Window = CreateWindowExA(0,
                                 WindowClass.lpszClassName,
                                 "Haw Yeah",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 Settings.ResWidth,
                                 Settings.ResHeight,
                                 0,
                                 0,
                                 hInstance,
                                 0);

        if (!Window)
        {
            // TODO: Display error message
            InvalidCodePath;
        }
        
        GlobalState.DeviceContext = GetDC(Window);
        GlobalGLContext = Win32InitOpenGL();
    }
    
    // NOTE: Init Input
    game_input GameInput = {};
    {
        GameInput.MouseP = V2(2, 2);
    }
    
    //
    // NOTE: Setup game executable
    //

    win32_game_code GameCode = {};
    {
        char SourceDLLPath[256] = "W:\\kiwi\\build_win32\\kiwi.dll";
        GameCode.SourceDLLPath = SourceDLLPath;

        char TempDLLPath[256] = "W:\\kiwi\\build_win32\\kiwi_temp.dll";
        GameCode.TempDLLPath = TempDLLPath;

        char LockFilePath[256] = "W:\\kiwi\\build_win32\\lock.tmp";
        GameCode.LockFilePath = LockFilePath;

        Win32LoadGameCode(&GameCode);
    }
    
    //
    // NOTE: Game memory + Init
    //

    void* ProgramMemory = 0;
    game_memory GameMem = {};
    game_render_commands* Commands = 0;
    {
        GameMem.PermanentMemSize = MegaBytes(100);
        mm ProgMemSize = GameMem.PermanentMemSize;

        LPVOID BaseAddress = (LPVOID)TeraBytes(2);
        
        ProgramMemory = VirtualAlloc(BaseAddress, ProgMemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!ProgramMemory)
        {
            InvalidCodePath;
        }
        
        mem_arena ProgArena = InitArena(ProgramMemory, ProgMemSize);
        GameMem.PermanentMem = PushSize(&ProgArena, GameMem.PermanentMemSize);
        GameMem.PushTextureToScreen = GLPushTextureToScreen;
        Win32ReadAssetFile(&GameMem);
        
        InitOpenGL(512, 512);
    }

    //
    // NOTE: Game debug
    //

    LARGE_INTEGER TimerFrequency;
    QueryPerformanceFrequency(&TimerFrequency);
    GlobalState.TimerFrequency = TimerFrequency.QuadPart;
    
    LARGE_INTEGER LastFrameTime = Win32GetClock();
            
    while (GlobalState.GameIsRunning)
    {
        if (GameMem.IsInitialized && GameMem.AssetMem)
        {
            VirtualFree(GameMem.AssetMem, 0, MEM_RELEASE);
            GameMem.AssetMem = 0;
        }
        
        // NOTE: Reload game code DLL if the DLL changed
        FILETIME NewDLLFileTime = Win32GetLastFileTime(GameCode.SourceDLLPath);
        if (CompareFileTime(&NewDLLFileTime, &GameCode.LastDLLFileTime) != 0)
        {
            Win32LoadGameCode(&GameCode);
        }
        
        // NOTE: Input
        b32 QPressed = false;
        b32 WPressed = false;
        {
            //TIMED_BLOCK("Win32 Input");
            
            POINT MousePos;
            if (!GetCursorPos(&MousePos))
            {
                InvalidCodePath;
            }
            
            RECT WindowRect;
            GetClientRect(Window, &WindowRect);
            ScreenToClient(Window, &MousePos);
            GameInput.MouseP.x = ((f32)(MousePos.x - WindowRect.left) /
                              (f32)(WindowRect.right - WindowRect.left));
            GameInput.MouseP.y = ((f32)(MousePos.y - WindowRect.bottom) /
                              (f32)(WindowRect.top - WindowRect.bottom));
            GameInput.MouseP = (2.0f*GameInput.MouseP) - V2(1, 1);

            if (GameInput.MouseP.x > 1.0f || GameInput.MouseP.y > 1.0f ||
                GameInput.MouseP.x < -1.0f || GameInput.MouseP.y < -1.0f)
            {
                GameInput.MouseDown = false;
            }
            else
            {
                b32 IsLeftMouseDown = GetKeyState(VK_LBUTTON) & (1 << 15);
                b32 IsRightMouseDown = GetKeyState(VK_RBUTTON) & (1 << 15);

                GameInput.MouseDown = IsLeftMouseDown || IsRightMouseDown;
            }

            MSG Message;
            while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
            {
                switch (Message.message)
                {
                    case WM_QUIT:
                    {
                        GlobalState.GameIsRunning = false;
                    } break;

                    case WM_SYSKEYDOWN:
                    case WM_SYSKEYUP:
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    {
                        u32 VKCode = (u32)Message.wParam;
                        b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                        b32 IsDown = ((Message.lParam & (1 << 31)) == 0);

                        switch (VKCode)
                        {
                            case 'W':
                            {
                                GameInput.MoveUpDown = IsDown;
                            } break;

                            case 'S':
                            {
                                GameInput.MoveDownDown = IsDown;
                            } break;

                            case 'A':
                            {
                                GameInput.MoveLeftDown = IsDown;
                            } break;

                            case 'D':
                            {
                                GameInput.MoveRightDown = IsDown;
                            } break;
                        }
                    } break;
                    
                    default:
                    {
                        TranslateMessage(&Message);
                        DispatchMessageA(&Message);
                    }
                }
            }
        }
        
        // NOTE: Update game logic and render prep
        {
            //TIMED_BLOCK("Win32 Logic and RenderPrep");
            
            if (!GameCode.UpdateAndRender)
            {
                // TODO: This should only happen in debug, for release compile game code into same exe
                InvalidCodePath;
            }

            GameCode.UpdateAndRender(&GameMem, &GameInput, Settings);
        }

        {
            {
                //TIMED_BLOCK("Blit To Screen");
                RECT WinRect;
                if (!GetClientRect(Window, &WinRect))
                {
                    InvalidCodePath;
                }
                i32 WinWidth = WinRect.right - WinRect.left;
                i32 WinHeight = WinRect.bottom - WinRect.top;

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, WinWidth, WinHeight);

                glBindTexture(GL_TEXTURE_2D, OpenGL.FrameTexture);
                glBindBuffer(GL_ARRAY_BUFFER, OpenGL.SquareVBO);
           
                BeginProgram(OpenGL.RenderFrameProg);
                glDrawArrays(GL_TRIANGLES, 0, 3*2);
                EndProgram(OpenGL.RenderFrameProg);
                
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            {
                //TIMED_BLOCK("Swap Buffers");
                SwapBuffers(GlobalState.DeviceContext);
                GLCheckError();
            }
        }
    }
    
    return 0;
}
