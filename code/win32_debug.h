#ifndef WIN32_MULTIMA_DEBUG_H

extern void Log(const char *);

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
Log(const char *Text)
{
#if defined(MULTIMA_DEBUG)
    if (!GameConsole.Init) {
        AllocConsole();
        GameConsole.OutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        GameConsole.Init = true;
    }

    DWORD Written;
    WriteConsole(GameConsole.OutHandle, Text, strlen(Text), &Written, 0);
#endif
}

#define WIN32_MULTIMA_DEBUG_H
#endif
