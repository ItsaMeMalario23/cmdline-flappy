#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <screen.h>
#include <render.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

u64 g_screenState = 0;

char g_screenBuf[SCR_BUF_SIZE];
char g_screenErrBuf[SCR_ERR_BUF_SIZE];

//
//  Local functions
//
static inline void setScreenIdx(u64 index)
{
    g_screenState = (g_screenState & (~ 0xffff)) | index;
}

static inline void setScreenState(u64 state)
{
    g_screenState = (g_screenState & (~ 0xffff0000)) | state << 16;
}

static inline void setScreenLines(u64 lines)
{
    g_screenState = (g_screenState & (~ 0xffff00000000)) | lines << 32;
}

static inline u16 getScreenIdx(void)
{
    return g_screenState;
}

static inline u16 _getScreenState(void)
{
    return g_screenState >> 16;
}

static inline u16 getScreenLines(void)
{
    return g_screenState >> 32;
}

//
//  Public functions
//
u16 getScreenState(void)
{
    return _getScreenState();
}

char* getScreenBuf(void)
{
    return g_screenBuf;
}

void screenResetBuf(void)
{
    g_screenState = 0;
}

void screenSetErrorBuf(const char* msg)
{
    strncpy(g_screenErrBuf, msg, SCR_ERR_BUF_SIZE);
    g_screenErrBuf[SCR_ERR_BUF_SIZE - 1] = '\0';
}

void screenBuildTextLine(const char* line, u8 ctrlChars)
{
    rAssert(getScreenIdx() < SCR_BUF_SIZE - SCREEN_WIDTH);
    rAssertMsg(getScreenState() != SCR_EMPTY, "Build text line: screen buf empty");
    rAssertMsg(getScreenState() != SCR_ERROR, "Build text line: screen buf error");
    rAssertMsg(getScreenState() != SCR_READY, "Build text line: screen buf full");

    char linebuf[SCREEN_WIDTH * 2];

    rAssert(ctrlChars < SCREEN_WIDTH);

    size_t len = SCREEN_WIDTH - 4 + ctrlChars;

    memset(linebuf, 32, sizeof(char) * len);
    memcpy(linebuf, line, sizeof(char) * strnlen(line, len));

    linebuf[len] = '\0';

    u16 idx = getScreenIdx();

    setScreenIdx(idx + snprintf(g_screenBuf + idx, SCR_BUF_SIZE - idx, SCR_BORDER_STR " %s " SCR_BORDER_STR, linebuf));
    setScreenLines(getScreenLines() + 1);
}

void screenBuildPixelLine(const u8* pxIds, size_t pxLen)
{
    rAssert(pxIds);
    rAssert(pxLen <= (SCREEN_WIDTH - 4) / 2);

    rAssertMsg(getScreenState() != SCR_EMPTY, "Build pixel line: screen buf empty");
    rAssertMsg(getScreenState() != SCR_ERROR, "Build pixel line: screen buf error");
    rAssertMsg(getScreenState() != SCR_READY, "Build pixel line: screen buf full");

    char linebuf[4096];

    u16 idx = 0;
    i16 col = -1;

    for (u8 i = 0; i < pxLen; i++) {
        if (pxIds[i] != col && pxIds[i] < 16) {
            idx += snprintf(linebuf + idx, sizeof(linebuf) - idx, PXL_RESET "%s  ", g_rPixelColors[pxIds[i]]);
        } else {
            linebuf[idx++] = 32;
            linebuf[idx++] = 32;
        }

        col = pxIds[i];

        rAssert(idx < sizeof(linebuf));
    }

    memcpy(linebuf + idx, PXL_RESET, sizeof(char) * 14);
    idx += 14;

    col = (WORLD_WIDTH - pxLen) * 2;

    if (col > 0) {
        memset(linebuf + idx, 32, sizeof(char) * col);
        linebuf[idx + col] = '\0';
    } else {
        linebuf[idx] = '\0';
    }

    idx = getScreenIdx();

    setScreenIdx(idx + snprintf(g_screenBuf + idx, SCR_BUF_SIZE - idx, SCR_BORDER_STR " %s " SCR_BORDER_STR, linebuf));
    setScreenLines(getScreenLines() + 1);
}

void screenBuildEmptyLine(void)
{
    rAssert(getScreenIdx() < SCR_BUF_SIZE - SCREEN_WIDTH);
    rAssertMsg(getScreenState() != SCR_EMPTY, "Build empty line: screen buf empty");
    rAssertMsg(getScreenState() != SCR_ERROR, "Build empty line: screen buf error");
    rAssertMsg(getScreenState() != SCR_READY, "Build empty line: screen buf full");

    u16 idx = getScreenIdx();

    memset(g_screenBuf + idx, 32, sizeof(char) * SCREEN_WIDTH);

    g_screenBuf[idx] = '|';
    g_screenBuf[idx + (SCREEN_WIDTH - 1)] = '|';

    setScreenIdx(idx + SCREEN_WIDTH);
    setScreenLines(getScreenLines() + 1);
}

void screenBuildPadding(u8 numLines)
{
    rWarning(numLines);

    for (u8 i = 0; i < numLines; i++)
        screenBuildEmptyLine();
}

void screenBuildHeader(void)
{
    rAssert(!getScreenState());
    rAssert(!getScreenLines());
    rAssert(!getScreenIdx());

    memcpy(g_screenBuf, TXT_RESET, sizeof(char) * 14);
    memset(g_screenBuf + 14, '-', sizeof(char) * SCREEN_WIDTH);
    memset(g_screenBuf + SCREEN_WIDTH + 15, 32, sizeof(char) * (SCREEN_WIDTH - 2));

    g_screenBuf[14] = '+';
    g_screenBuf[SCREEN_WIDTH + 13] = '+';
    g_screenBuf[SCREEN_WIDTH + 14] = '|';
    g_screenBuf[(SCREEN_WIDTH * 2) + 13] = '|';

    setScreenIdx(334);
    setScreenLines(2);
    setScreenState(SCR_INCOMPLETE);
}

void screenBuildHeaderFPS(f64 dt)
{
    rAssert(!getScreenState());
    rAssert(!getScreenLines());
    rAssert(!getScreenIdx());

    memcpy(g_screenBuf, TXT_RESET, sizeof(char) * 14);
    memset(g_screenBuf + 14, '-', sizeof(char) * SCREEN_WIDTH);
    memset(g_screenBuf + SCREEN_WIDTH + 15, 32, sizeof(char) * (SCREEN_WIDTH - 2));

    char fpsbuf[40];

    if (dt > 0.000000000001f)
        snprintf(fpsbuf, 40, "< FPS: %.2f >----< MEM: %lld B >", 1 / dt, g_memAllocated);
    else
        snprintf(fpsbuf, 40, "< FPS:  N/A  >----< MEM: %lld B >", g_memAllocated);

    memcpy(g_screenBuf + 134, fpsbuf, strnlen(fpsbuf, 40));

    g_screenBuf[14] = '+';
    g_screenBuf[SCREEN_WIDTH + 13] = '+';
    g_screenBuf[SCREEN_WIDTH + 14] = '|';
    g_screenBuf[(SCREEN_WIDTH * 2) + 13] = '|';

    setScreenIdx(334);
    setScreenLines(2);
    setScreenState(SCR_INCOMPLETE);
}

void screenBuildFooter(void)
{
    rAssert(getScreenIdx() < SCR_BUF_SIZE - SCREEN_WIDTH);
    rAssert(getScreenState() == SCR_INCOMPLETE);

    u16 lines = getScreenLines();
    u16 idx = getScreenIdx();

    while (lines++ < SCREEN_HEIGHT - 2) {
        rAssert(getScreenIdx() < SCR_BUF_SIZE - SCREEN_WIDTH);

        memset(g_screenBuf + idx, 32, sizeof(char) * SCREEN_WIDTH);

        g_screenBuf[idx] = '|';
        g_screenBuf[idx + (SCREEN_WIDTH - 1)] = '|';

        idx += SCREEN_WIDTH;
    }

    rAssert(idx < SCR_BUF_SIZE - (SCREEN_WIDTH + 1));

    memset(g_screenBuf + idx, '-', sizeof(char) * SCREEN_WIDTH);

    g_screenBuf[idx] = '+';
    g_screenBuf[idx + SCREEN_WIDTH - 1] = '+';
    g_screenBuf[idx + SCREEN_WIDTH] = '\0';
    
    setScreenState(SCR_READY);
}

// temp

void testScr(void)
{
    printf("idx: %d state: %d lines %d\n", getScreenIdx(), getScreenState(), getScreenLines());
    setScreenIdx(23);
    printf("idx: %d state: %d lines %d\n", getScreenIdx(), getScreenState(), getScreenLines());
    setScreenState(SCR_READY);
    setScreenLines(69);
    printf("idx: %d state: %d lines %d\n", getScreenIdx(), getScreenState(), getScreenLines());
    setScreenLines(0);
    printf("idx: %d state: %d lines %d\n", getScreenIdx(), getScreenState(), getScreenLines());
    setScreenIdx(0);
    setScreenState(0);
    printf("idx: %d state: %d lines %d\n", getScreenIdx(), getScreenState(), getScreenLines());
    setScreenIdx(15999);
    setScreenState(SCR_ERROR);
    setScreenLines(48);
    printf("idx: %d state: %d lines %d\n", getScreenIdx(), getScreenState(), getScreenLines());
}