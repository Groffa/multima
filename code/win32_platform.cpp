#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "game.h"
#include "game_memory.h"
#include "game_basictypes.h"
#include "win32_debug.h"

LRESULT CALLBACK GameWindowCallback(HWND, UINT, WPARAM, LPARAM);

gamestate_t GameState = {0};

uint
MapFile(const char *Filename, gamememory_t *Memory)
{
    // If Memory is null or of size zero, just return file's size
    LARGE_INTEGER FileSize = {0};
    HANDLE FileHandle = CreateFile(Filename, GENERIC_READ, 
            FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (FileHandle == INVALID_HANDLE_VALUE) {
        assert(!"File not found");
    }
    GetFileSizeEx(FileHandle, &FileSize);
    if (Memory && Memory->Size >= FileSize.QuadPart) {
        char *Source = (char *)malloc(FileSize.QuadPart);
        DWORD Read;
        ReadFile(FileHandle, Source, FileSize.QuadPart, &Read, 0);
        // TODO: MemoryType_char isn't quite right, but there's no datatype for this
        CopyMemory(Memory->Data, Source, FileSize.QuadPart);
        free(Source);
    } else {
        assert(!"File too big");
    }
    CloseHandle(FileHandle);
    return FileSize.QuadPart;
}

static WNDCLASSEX
RegisterGameWindowClass(HINSTANCE instance)
{
    WNDCLASSEX cls = {0};
    
    cls.cbSize = sizeof(WNDCLASSEX);
    cls.style = CS_HREDRAW | CS_VREDRAW;
    cls.lpfnWndProc = GameWindowCallback;
    cls.hInstance = instance;
    cls.lpszClassName = GAME_TITLE;
    cls.hIcon = LoadIcon(0, IDI_APPLICATION); 
    RegisterClassEx(&cls);

    return cls;
}

static void
AllocateGameMemory(gamestate_t *GameState, uint64 Size)
{
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    LOGF("Allocating %d. Page size of computer is %d.", Size, SystemInfo.dwPageSize);

    GameState->Memory.Size = Size;
    GameState->Memory.Data = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

static void
DeallocateGameMemory(gamestate_t *GameState)
{
    VirtualFree(GameState->Memory.Data, 0, MEM_RELEASE);
    GameState->Memory.Size = 0;
}

static HWND
CreateGameWindow()
{
    HWND wnd = {0};
    HINSTANCE instance = GetModuleHandleA(0);
    RegisterGameWindowClass(instance);
    wnd = CreateWindowA(GAME_TITLE, GAME_TITLE,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,   // x & y
            CW_USEDEFAULT, CW_USEDEFAULT,   // width & height
            0, 0,
            instance,
            0);

    return wnd;
}

static
gameapi_t LoadGame()
{
    gameapi_t api = {0};
    
#if defined(MULTIMA_DEBUG)
    api.Handle = LoadLibraryA("game_debug.dll");
#else
    api.Handle = LoadLibraryA("game.dll");
#endif
    assert(api.Handle != 0);
   
    api.Log = Log;
    api.MapFile = MapFile;

    api.RunFrame = (runframe_f) GetProcAddress((HMODULE)api.Handle, "RunFrame");
    assert(api.RunFrame != 0);

    return api;
}

static void
UnloadGame(gameapi_t *api)
{
    if (api->Handle) {
        FreeLibrary((HMODULE) api->Handle);
        api->Handle = 0;
    }
    FreeConsole();
}

static LRESULT
GameWindowCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_ACTIVATE:
            GameState.Running = (LOWORD(wParam) != WA_INACTIVE);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(wnd, msg, wParam, lParam);
}

static void
UnloadScreenBuffer(gamestate_t *GameState)
{
    VirtualFree(GameState->DrawBuffer.Buffer, 0, MEM_RELEASE);
    GameState->DrawBuffer.Buffer = 0;
}

static BITMAPINFO bmi = {0};
static void
InitScreenBuffer(gamestate_t *GameState, HWND hwnd)
{
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = GameState->DrawBuffer.Width;
    bmi.bmiHeader.biHeight = -GameState->DrawBuffer.Height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // 4 is bytes per pixel (32 bits)
    const int BufferSize =
        GameState->DrawBuffer.Width * GameState->DrawBuffer.Height * 4;
    LPVOID memory = VirtualAlloc(0, BufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    GameState->DrawBuffer.Buffer = memory;
}

int 
WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int cmdshow)
{
    gameapi_t GameApi = LoadGame();
    
    HWND hwnd = CreateGameWindow();
    ShowWindow(hwnd, cmdshow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    
    GameState.DrawBuffer.Width = 640;
    GameState.DrawBuffer.Height = 480;
    InitScreenBuffer(&GameState, hwnd);

    const uint MemorySize = MEGABYTES(16);

    AllocateGameMemory(&GameState, MemorySize);

    float Scale = 2.0f;
    
    LARGE_INTEGER StartCounter = {0};
    uint64 FrameTargetMS = 16;  // 60 FPS

    timeBeginPeriod(1);
    LARGE_INTEGER Frequency; 
    QueryPerformanceFrequency(&Frequency);
    
    bool GameRunning = true;
    while (GameRunning) {
        QueryPerformanceCounter(&StartCounter);
        
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
                case WM_QUIT:
                    GameRunning = false;
                    break;

                default:
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
            }
        }

#if defined(MULTIMA_DEBUG)
        ClearLog();
#endif
        GameApi.RunFrame(&GameApi, &GameState);

        HDC hdc = GetDC(hwnd);
        const int res = StretchDIBits(
            hdc,
            0,
            0,
            (uint)(Scale * GameState.DrawBuffer.Width),
            (uint)(Scale * GameState.DrawBuffer.Height), 
            0,
            0,
            GameState.DrawBuffer.Width,
            GameState.DrawBuffer.Height, 
            GameState.DrawBuffer.Buffer,
            &bmi,
            DIB_RGB_COLORS,
            SRCCOPY
        );
        ReleaseDC(hwnd, hdc);

        LARGE_INTEGER EndCounter = {0};
        QueryPerformanceCounter(&EndCounter);
        GameState.DeltaTime = EndCounter.QuadPart - StartCounter.QuadPart;
        uint64 FrameTimeMS = (1000 * GameState.DeltaTime) / Frequency.QuadPart;

        if (FrameTimeMS < FrameTargetMS) {
            Sleep(FrameTargetMS - FrameTimeMS);
        }

        char WindowTitle[128] = {0};
        _snprintf(WindowTitle, sizeof(WindowTitle),
                "%s - %3d ms / %4d ticks",
                GAME_TITLE,
                FrameTimeMS, GameState.DeltaTime);
        SetWindowText(hwnd, WindowTitle);
    }
    UnloadGame(&GameApi);
    UnloadScreenBuffer(&GameState);

    DeallocateGameMemory(&GameState);

    timeEndPeriod(1);
   
    return 0;
}
