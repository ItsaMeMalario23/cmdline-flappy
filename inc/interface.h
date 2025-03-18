#ifndef INTERFACE_H
#define INTERFACE_H

#include <main.h>

char getKeyboardInput(void);
char awaitKeyboardInput(void);

void menuController(menu_t* menu);
void flappyController(u8 context);
void settingsController(u8 context);

void buildSettingsMenuItems(char buf[][64], const char** align, size_t size);

#endif