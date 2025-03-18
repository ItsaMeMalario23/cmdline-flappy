#include <stdio.h>
#include <stdlib.h>

#include <main.h>
#include <render.h>
#include <interface.h>

const char* mainMenuItems[4] = {
    "Start",
    "Highscores",
    "Settings",
    "Quit"
};

void dummyCallback(u8 context) { }

void exitCallback(u8 context) { exit(0); }

int main(void)
{
    if (!initCmdRenderer())
        return EXIT_FAILURE;

    const nav_f mainNav[4] = {flappyController, dummyCallback, settingsController, exitCallback};

    menu_t mainMenu = {"MAIN MENU", mainMenuItems, mainNav, 0, 4};

    menuController(&mainMenu);

    return EXIT_SUCCESS;
}