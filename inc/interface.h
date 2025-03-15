#ifndef INTERFACE_H
#define INTERFACE_H

#include <main.h>

char getKeyboardInput(void);
char awaitKeyboardInput(void);

void menuController(menu_t* menu);
void gameController(u8 context);
void flappyController(u8 context);

#endif