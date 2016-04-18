#ifndef WIN32_MULTIMA_DEBUG_H

#if defined(MULTIMA_DEBUG)
    #define LOGF(Text, ...) \
        { \
          char localBuff[255] = {0}; \
          _snprintf(localBuff, sizeof(localBuff), Text, __VA_ARGS__); \
          Log(localBuff); \
        }
#else
    #define LOGF(Text, ...)
#endif

static struct 
{
    bool Init;
    HANDLE OutHandle;
} GameConsole = {0};

static void
InitConsole()
{
#if 0
    if (!GameConsole.Init) {
        AllocConsole();
        GameConsole.OutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        GameConsole.Init = true;
    }
#endif
}

void
Log(const char *Text)
{
#if defined(MULTIMA_DEBUG)
    DWORD Written;
    InitConsole();
    WriteConsole(GameConsole.OutHandle, Text, strlen(Text), &Written, 0);
#endif
}

void
ClearLog()
{
#if defined(MULTIMA_DEBUG)
    DWORD Written;
    COORD coord = {0, 0};
    InitConsole();
    FillConsoleOutputCharacter(GameConsole.OutHandle, ' ', 80*25, coord, &Written);
    SetConsoleCursorPosition(GameConsole.OutHandle, coord);
#endif
}

#define WIN32_MULTIMA_DEBUG_H
#endif
