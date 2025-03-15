#include <stdio.h>
#include <stdlib.h>

#include <main.h>
#include <render.h>
#include <interface.h>

const char* mainMenuItems[3] = {
    "Start",
    "Highscores",
    "Quit"
};

void dummyCallback(u8 context) { }

void exitCallback(u8 context) { exit(0); }

int main(void)
{
    if (!initCmdRenderer())
        return EXIT_FAILURE;

    const nav_f mainNav[3] = {flappyController, dummyCallback, exitCallback};

    menu_t mainMenu = {"MAIN MENU", mainMenuItems, mainNav, 0, 3};

    menuController(&mainMenu);

    return EXIT_SUCCESS;
}