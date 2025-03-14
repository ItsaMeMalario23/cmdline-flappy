#ifndef SCREEN_H
#define SCREEN_H

#include <main.h>

#define SCR_BORDER_CH   ('+')
#define SCR_BORDER_STR  "|"

char* getScreenBuf(void);
u16 getScreenState(void);
void screenResetBuf(void);

void screenSetErrorBuf(const char* msg);

void screenBuildTextLine(const char* line, u8 ctrlChars);
void screenBuildPixelLine(const u8* pxIds, size_t pxLen);
void screenBuildEmptyLine(void);
void screenBuildPadding(u8 numLines);
void screenBuildHeaderFPS(f64 dt);
void screenBuildHeader(void);
void screenBuildFooter(void);

#endif