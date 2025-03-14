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

void gameController(u8 context)
{
    initWorld();

    (void) addPipe(7, 5, 3);
    (void) addPipe(4, 42, 14);
    (void) addPipe(6, 23, 34);
    sprite_t* pipe = addPipe(50, 64, 1);
    sprite_t* bird = addBird(23, 7);

    bool needRebuild = 1;

    char input;

    clock_t prevt = clock();
    clock_t currt;

    f64 dt;

    setFrametime(0.01f);

    for (;;) {
        if (needRebuild)
            updateWorld();

        currt = clock();
        dt = (f64) (currt - prevt) / CLOCKS_PER_SEC;
        prevt = currt;

        if (dt > 0.000000000001f)
            setFrametime(dt);

        input = awaitKeyboardInput();
        
        if (!input)
            continue;

        needRebuild = 1;

        if (input == 'w' || input == 'W')
            moveSprite(bird, 0, -1);
        else if (input == 'a' || input == 'A')
            moveSprite(bird, -1, 0);
        else if (input == 's' || input == 'S')
            moveSprite(bird, 0, 1);
        else if (input == 'd' || input == 'D')
            moveSprite(bird, 1, 0);
        else if (input == 'z')
            moveSprite(pipe, 0, -1);
        else if (input == 'h')
            moveSprite(pipe, 0, 1);
        else if (input == 27)
            break;     // esc
        else
            needRebuild = 0;
    }

    cleanupWorld();
}