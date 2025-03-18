#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <windows.h>
#include <conio.h>

#include <interface.h>
#include <render.h>
#include <worldsim.h>
#include <debug/rdebug.h>

char getKeyboardInput(void)
{
    char input = 0;

    if (_kbhit())
        input = _getch();

    fflush(stdin);

    return input;
}

char awaitKeyboardInput(void)
{
    fflush(stdin);

    while (!_kbhit());

    return _getch();
}

void menuController(menu_t* menu)
{
    rAssert(menu);
    rAssert(menu->items);
    rAssert(menu->navcom);
    rAssert(menu->len);

    i8 diff = 0;
    i8 selected = 0;
    bool needRebuild = 1;

    char input;

    for (;;) {
        rAssert(selected + diff >= 0);

        if (needRebuild)
            renderMenu(menu, selected = selected + diff);

        input = awaitKeyboardInput();

        needRebuild = 0;
        diff = 0;

        if ((input == 'w' || input == 'W') && selected)
        {
            diff = -1;
            needRebuild = 1;
        }
        else if ((input == 's' || input == 'S') && selected < menu->len - 1)
        {
            diff = 1;
            needRebuild = 1;
        }
        else if (input == '\r')
        {
            if (!menu->navcom[selected])
                return;

            menu->navcom[selected](menu->navkey);

            diff = 0;
            needRebuild = 1;
        }
        else if (input == 27)
        {
            return;     // esc
        }
    }
}

// flappy game loop
void flappyController(u8 context)
{
    initWorld();

    bool updraft;
    char input;

    clock_t prevt = clock();
    clock_t currt;

    updateWorld(0, 0);

    for (;;) {
        currt = clock();

        while (currt - prevt < FIXED_FRAME_TIME)
            currt = clock();

        setFrametime(currt - prevt);

        input = getKeyboardInput();

        if (input == 27)
            break;  // esc
        else if (input == 'w' || input == 32)
            updraft = 1;
        else
            updraft = 0;

        requestWorldRebuild();

        if (updateWorld(currt - prevt, updraft) > 0)
            break;

        prevt = currt;
    }

    cleanupWorld();
}

const char* settingsMenuItems_fmt[10] = {
    "Gravity:               %s<< %3.1f >>",
    "Updraft strength:      %s<< %3.1f >>",
    "Updraft damping:       %s<< %3.2f >>",
    "Scroll interval:       %s<< %4d >>",
    "Bird X position:       %s<< %4.0f >>",
    "Bird Y position:       %s<< %4.0f >>",
    "First pipe distance:   %s<< %4d >>",
    "Pipe distance:         %s<< %4d >>",
    "Reset all to default",
    "Back"
};

const nav_f settingsNav[10] = {
    incrementGravity,
    incrementUpdraftStrength,
    incrementUpdraftDamping,
    incrementScrollInterval,
    incrementBirdXPos,
    incrementBirdYPos,
    incrementFirstPipe,
    incrementPipeDistance,
    setDefaults,
    NULL
};

// settings menu controller
void settingsController(u8 context)
{
    char menuBuf[10][64];

    const char* pMenuBuf[10] = {
        menuBuf[0], menuBuf[1], menuBuf[2], menuBuf[3], menuBuf[4],
        menuBuf[5], menuBuf[6], menuBuf[7], menuBuf[8], menuBuf[9]
    };

    menu_t menu = {"SETTINGS", pMenuBuf, settingsNav, 0, 10};

    i8 diff = 0;
    i8 selected = 0;
    bool needRebuild = 1;

    char input;

    const char* menuAlign[8];
    const char* sp = " ";
    const char* nsp = "\0";

    for (;;) {
        selected += diff;

        rAssert(selected >= 0);

        for (u8 i = 0; i < 8; i++)
            menuAlign[i] = sp;

        if (selected < 8)
            menuAlign[selected] = nsp;

        buildSettingsMenuItems(menuBuf, menuAlign, 64);

        if (needRebuild)
            renderMenu(&menu, selected);

        input = awaitKeyboardInput();

        needRebuild = 0;
        diff = 0;

        if ((input == 'w' || input == 'W') && selected)
        {
            diff = -1;
            needRebuild = 1;
        }
        else if ((input == 's' || input == 'S') && selected < menu.len - 1)
        {
            diff = 1;
            needRebuild = 1;
        }
        else if ((input == 'a' || input == 'A') && selected < menu.len - 2)
        {
            if (menu.navcom[selected])
                menu.navcom[selected](SETTINGS_DECREASE);
            else
                return;

            needRebuild = 1;
        }
        else if ((input == 'd' || input == 'D') && selected < menu.len - 2)
        {
            if (menu.navcom[selected])
                menu.navcom[selected](SETTINGS_INCREASE);
            else
                return;

            needRebuild = 1;
        }
        else if (input == '\r')
        {
            if (menu.navcom[selected])
                menu.navcom[selected](SETTINGS_NOP);
            else
                return;

            needRebuild = 1;
        }
        else if (input == 27)
        {
            return;     // esc
        }
    }
}

void buildSettingsMenuItems(char buf[][64], const char** align, size_t size)
{
    snprintf(buf[0], size, settingsMenuItems_fmt[0], align[0], getGravity());
    snprintf(buf[1], size, settingsMenuItems_fmt[1], align[1], getUpdraftStrength());
    snprintf(buf[2], size, settingsMenuItems_fmt[2], align[2], getUpdraftDamping());
    snprintf(buf[3], size, settingsMenuItems_fmt[3], align[3], getScrollInterval());
    snprintf(buf[4], size, settingsMenuItems_fmt[4], align[4], getBirdXPos());
    snprintf(buf[5], size, settingsMenuItems_fmt[5], align[5], getBirdYPos());
    snprintf(buf[6], size, settingsMenuItems_fmt[6], align[6], getFirstPipeDistance());
    snprintf(buf[7], size, settingsMenuItems_fmt[7], align[7], getPipeDistance());
    strncpy(buf[8], settingsMenuItems_fmt[8], size);
    strncpy(buf[9], settingsMenuItems_fmt[9], size);
}