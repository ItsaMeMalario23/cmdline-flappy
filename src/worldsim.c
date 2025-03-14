#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <worldsim.h>
#include <render.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

u8 g_worldState = WORLD_UNINIT;
u8 g_worldRenderBuf[WORLD_HEIGHT][WORLD_WIDTH];

sprite_t g_sprites[WORLD_MAX_SPRITES];
u8 g_spriteIdx = 0;

const u8 g_pipeData[4][8] = {
    { 2,  2,  2,  2,  2,  2,  2,  2},
    { 2, 10, 10, 10, 15, 10, 10,  2},
    { 2,  2, 10, 10, 15, 10,  2,  2},
    {16,  2, 10, 10, 15, 10,  2, 16}
};

const u8 g_birdData[40] = {
    16, 16,  3,  3,  3,  3,  3, 16,
    16,  3,  3, 15,  0,  3,  0, 16,
     3,  3,  3,  3,  3,  1,  1,  3,
    16,  3,  3,  3,  1,  1,  1,  1,
    16, 16,  3,  3,  3,  3,  3, 16,
};

sprite_t* addSprite(u16 width, u16 height, u16 xpos, u16 ypos)
{
    if (g_spriteIdx > WORLD_MAX_SPRITES - 1)
        return NULL;

    rAssert(width);
    rAssert(height);

    sprite_t* sprite = g_sprites + g_spriteIdx;

    g_spriteIdx++;

    sprite->width = width;
    sprite->height = height;
    sprite->xpos = xpos;
    sprite->ypos = ypos;

    sprite->data = memAlloc(sizeof(u8) * width * height);

    rAssert(sprite->data);

    memset(sprite->data, PXID_CLEAR, sizeof(u8) * width * height);

    g_worldState = WORLD_NEED_REBUILD;
    
    return sprite;
}

sprite_t* addPipe(u32 height, u16 xpos, u16 ypos)
{
    sprite_t* sprite = addSprite(8, height + 4, xpos, ypos);

    if (!sprite)
        return NULL;

    memcpy(sprite->data, g_pipeData[0], sizeof(u8) * 8);
    memcpy(sprite->data + 8, g_pipeData[1], sizeof(u8) * 8);
    memcpy(sprite->data + 16, g_pipeData[1], sizeof(u8) * 8);
    memcpy(sprite->data + 24, g_pipeData[2], sizeof(u8) * 8);

    for (u8 i = 0; i < height; i++)
        memcpy(sprite->data + 24 + i * 8, g_pipeData[3], sizeof(u8) * 8);

    return sprite;
}

sprite_t* addBird(u16 xpos, u16 ypos)
{
    sprite_t* sprite = addSprite(8, 5, xpos, ypos);

    if (!sprite)
        return NULL;

    memcpy(sprite->data, g_birdData, sizeof(u8) * 40);

    return sprite;
}

void moveSprite(sprite_t* sprite, i16 dx, i16 dy)
{
    if (!sprite || (!dx && !dy))
        return;

    sprite->xpos += dx;
    sprite->ypos += dy;

    g_worldState = WORLD_NEED_REBUILD;
}

void renderSprite(sprite_t* sprite)
{
    if (!sprite)
        return;

    if (sprite->xpos + sprite->width < 0 || sprite->xpos >= WORLD_WIDTH)
        return;

    if (sprite->ypos + sprite->height < 0 || sprite->ypos >= WORLD_HEIGHT)
        return;

    u32 len = sprite->width * sprite->height;
    u8 *dst, *src;

    for (u16 y = 0; y < sprite->height; y++) {
        if (sprite->ypos + y < 0)
            continue;

        if (sprite->ypos + y >= WORLD_HEIGHT)
            break;

        dst = &g_worldRenderBuf[sprite->ypos + y][sprite->xpos];
        src = &sprite->data[y * sprite->width];

        for (u16 x = 0; x < sprite->width; x++) {
            if (*src == PXID_CLEAR) {
                dst++;
                src++;
                continue;
            }

            if (sprite->xpos + x < 0) {
                src++;
                continue;
            }
            
            if (sprite->xpos + x >= WORLD_WIDTH)
                break;

            *(dst++) = *(src++);
        }
    }
}

void initWorld(void)
{
    memset(g_sprites, 0, sizeof(g_sprites));
    memset(g_worldRenderBuf, PXID_WHITE, sizeof(g_worldRenderBuf));

    g_worldState = WORLD_NEED_REBUILD;
}

void cleanupWorld(void)
{
    for (u8 i = 0; i < g_spriteIdx; i++)
        memFree(g_sprites[i].data);

    memset(g_sprites, 0, sizeof(g_sprites));

    g_spriteIdx = 0;
    g_worldState = WORLD_UNINIT;
}

void requestWorldRebuild(void)
{
    if (g_worldState == WORLD_UNINIT)
        return;

    g_worldState = WORLD_NEED_REBUILD;
}

void updateWorld(void)
{
    if (g_worldState != WORLD_NEED_REBUILD)
        return;

    memset(g_worldRenderBuf, PXID_WHITE, sizeof(g_worldRenderBuf));

    for (u8 i = 0; i < g_spriteIdx; i++)
        renderSprite(&g_sprites[i]);

    renderWorld(g_worldRenderBuf);
    
    g_worldState = WORLD_NOP;
}