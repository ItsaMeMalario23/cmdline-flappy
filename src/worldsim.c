#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <worldsim.h>
#include <render.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

wstate_t g_worldState = WORLD_UNINIT;

u8 g_worldRenderBuf[WORLD_HEIGHT][WORLD_WIDTH];

sprite_t g_sprites[WORLD_MAX_SPRITES];
u8 g_spriteIdx = 0;

u64 g_scrollInterval = WORLD_BASE_SCROLL_INTERVAL;
i64 g_score = 0;

const texturedata_t g_pipeData[4][8] = {
    { 2,  2,  2,  2,  2,  2,  2,  2},
    { 2, 10, 10, 10, 15, 10, 10,  2},
    { 2,  2, 10, 10, 15, 10,  2,  2},
    {16,  2, 10, 10, 15, 10,  2, 16}
};

const texturedata_t g_birdData[40] = {
    16, 16,  3,  3,  3,  3,  3, 16,
    16,  3,  3, 15,  0,  3,  0, 16,
     3,  3,  3,  3,  3,  1,  1,  3,
    16,  3,  3,  3,  1,  1,  1,  1,
    16, 16,  3,  3,  3,  3,  3, 16,
};

sprite_t* addSprite(u16 width, u16 height, u16 posType, u16 spriteType, i32 xpos_i, i32 ypos_i, f32 xpos_f, f32 ypos_f)
{
    if (g_spriteIdx > WORLD_MAX_SPRITES - 1)
        return NULL;

    rAssert(width);
    rAssert(height);
    rAssert(posType == POS_TYPE_FLOAT || posType == POS_TYPE_INT);

    sprite_t* sprite = g_sprites + g_spriteIdx;

    g_spriteIdx++;

    sprite->width = width;
    sprite->height = height;
    sprite->posType = posType;
    sprite->spriteType = spriteType;

    if (posType == POS_TYPE_FLOAT) {
        sprite->pos.f.x = xpos_f;
        sprite->pos.f.y = ypos_f;
    } else {
        sprite->pos.i.x = xpos_i;
        sprite->pos.i.y = ypos_i;
    }

    sprite->data = memAlloc(sizeof(u8) * width * height);

    rAssert(sprite->data);

    memset(sprite->data, PXID_CLEAR, sizeof(u8) * width * height);

    g_worldState = WORLD_NEED_REBUILD;
    
    return sprite;
}

sprite_t* addPipe(i32 xpos, i32 ypos, u16 height)
{
    sprite_t* sprite = addSprite(8, height + 4, POS_TYPE_INT, SPRITE_PIPE, xpos, ypos, 0.0f, 0.0f);

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

sprite_t* addBird(f32 xpos, f32 ypos)
{
    sprite_t* sprite = addSprite(8, 5, POS_TYPE_FLOAT, SPRITE_BIRD, 0, 0, xpos, ypos);

    if (!sprite)
        return NULL;

    memcpy(sprite->data, g_birdData, sizeof(u8) * 40);

    return sprite;
}

void moveSprite(sprite_t* sprite, i32 dx_i, i32 dy_i, f32 dx_f, f32 dy_f)
{
    if (!sprite)
        return;

    if (sprite->posType == POS_TYPE_FLOAT)
        moveSpriteF(sprite, dx_f, dy_f);
    else
        moveSpriteI(sprite, dx_i, dy_i);
}

void moveSpriteI(sprite_t* sprite, i32 dx, i32 dy)
{
    if (!sprite || (!dx && !dy))
        return;

    rAssert(sprite->posType == POS_TYPE_INT);

    sprite->pos.i.x += dx;
    sprite->pos.i.y += dy;

    g_worldState = WORLD_NEED_REBUILD;
}

void moveSpriteF(sprite_t* sprite, f32 dx, f32 dy)
{
    if (!sprite || (dx < EPSILON && dx > -EPSILON && dy < EPSILON && dy > -EPSILON))
        return;

    rAssert(sprite->posType == POS_TYPE_FLOAT);

    sprite->pos.f.x += dx;
    sprite->pos.f.y += dy;

    g_worldState = WORLD_NEED_REBUILD;
}

void renderSprite(sprite_t* sprite)
{
    if (!sprite)
        return;

    rAssert(sprite->posType == POS_TYPE_FLOAT || sprite->posType == POS_TYPE_INT);

    i32 xpos, ypos;

    if (sprite->posType == POS_TYPE_FLOAT) {
        xpos = (i32) roundf(sprite->pos.f.x);
        ypos = (i32) roundf(sprite->pos.f.y);
    } else {
        xpos = sprite->pos.i.x;
        ypos = sprite->pos.i.y;
    }
    
    if (xpos + sprite->width < 0 || xpos >= WORLD_WIDTH)
        return;

    if (ypos + sprite->height < 0 || ypos >= WORLD_HEIGHT)
        return;

    texturedata_t *dst, *src;

    for (u16 y = 0; y < sprite->height; y++) {
        if (ypos + y < 0)
            continue;

        if (ypos + y >= WORLD_HEIGHT)
            break;

        dst = &g_worldRenderBuf[ypos + y][xpos];
        src = &sprite->data[y * sprite->width];

        for (u16 x = 0; x < sprite->width; x++) {
            if (*src == PXID_CLEAR) {
                dst++;
                src++;
                continue;
            }

            if (xpos + x < 0) {
                dst++;
                src++;
                continue;
            }
            
            if (xpos + x >= WORLD_WIDTH)
                break;

            *dst++ = *src++;
        }
    }
}

void initWorld(void)
{
    memset(g_sprites, 0, sizeof(g_sprites));
    memset(g_worldRenderBuf, PXID_WHITE, sizeof(g_worldRenderBuf));

    (void) addBird(WORLD_BIRD_STD_XPOS, 10.0f);
    (void) addPipe(40, 32, 20);
    (void) addPipe(70, 33, 20);
    (void) addPipe(100, 34, 20);
    (void) addPipe(130, 35, 20);

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

// return score on game over, -1 on game continue
i64 updateWorld(u64 dt, bool updraft)
{
    if (g_worldState != WORLD_NEED_REBUILD)
        return -1;

    static u64 scrollTimer = 0;
    static sprite_t* bird = NULL;

    if (!bird)
        bird = getBird();

    rAssert(bird);

    scrollTimer += dt;

    // scroll screen
    if (scrollTimer >= g_scrollInterval) {
        scrollScreen();
        scrollTimer = 0;

        if (g_scrollInterval > 30)
            g_scrollInterval -= 2;
    }

    // move bird
    handleBirdVerticalSpeed(bird, dt, updraft);

    // check bird collision
    if (checkCollision(bird))
        return g_score;

    g_score += 100;

    // render world
    memset(g_worldRenderBuf, PXID_WHITE, sizeof(g_worldRenderBuf));

    for (u8 i = 0; i < g_spriteIdx; i++)
        renderSprite(&g_sprites[i]);

    renderWorld(g_worldRenderBuf);
    
    g_worldState = WORLD_NOP;

    return -1;
}

//
//  Local functions
//

// find bird in sprites buffer
sprite_t* getBird(void)
{
    for (u8 i = 0; i < g_spriteIdx; i++) {
        if (g_sprites[i].spriteType == SPRITE_BIRD)
            return g_sprites + i;
    }

    return NULL;
}

// move all pipe sprites left by 1
void scrollScreen(void)
{
    sprite_t* tmp;

    for (u8 i = 0; i < g_spriteIdx; i++) {
        if (g_sprites[i].spriteType != SPRITE_PIPE)
            continue;

        tmp = g_sprites + i;

        rAssert(tmp->posType == POS_TYPE_INT);

        if (tmp->pos.i.x < -37)
            resetPipe(tmp);   // reset

        moveSpriteI(tmp, -1, 0);
    }
}

// reset pipe position to offscreen right
void resetPipe(sprite_t* pipe)
{
    rAssert(pipe);
    rAssert(pipe->posType == POS_TYPE_INT);
    rAssert(pipe->spriteType == SPRITE_PIPE);

    pipe->pos.i.x = WORLD_WIDTH + 1;
    pipe->pos.i.y = (rand() % (WORLD_HEIGHT - 5 - (WORLD_HEIGHT / 2))) + (WORLD_HEIGHT / 2);
}

// return true if bird collided with pipe
// search for bird in sprites buf if pointer is null
bool checkCollision(sprite_t* bird)
{
    if (!bird)
        bird = getBird();

    rAssert(bird);
    rAssert(bird->posType == POS_TYPE_FLOAT);
    rAssert(bird->spriteType == SPRITE_BIRD);
    rAssert(bird->pos.f.x + 0.01f > WORLD_BIRD_STD_XPOS && bird->pos.f.x - 0.01f < WORLD_BIRD_STD_XPOS);

    sprite_t* tmp;

    for (u8 i = 0; i < g_spriteIdx; i++) {
        tmp = g_sprites + i;

        if (tmp->spriteType != SPRITE_PIPE)
            continue;

        rAssert(tmp->posType == POS_TYPE_INT);

        // check for horizontal collision
        if (tmp->pos.i.x > WORLD_BIRD_STD_XPOS + bird->width || tmp->pos.i.x + tmp->width < WORLD_BIRD_STD_XPOS)
            continue;

        // check for vertical collision
        if (tmp->pos.i.y + tmp->height < bird->pos.f.y || tmp->pos.i.y > bird->pos.f.y + bird->height)
            continue;

        // found collision
        return 1;
    }

    // no collision found
    return 0;
}

// handle gravity and updraft input, can only handle vertical speed for one bird
// search for bird if pointer is null
void handleBirdVerticalSpeed(sprite_t* bird, u64 dt, bool updraft)
{
    if (!dt)
        return;

    if (!bird)
        bird = getBird();

    rAssert(bird);
    rAssert(bird->posType == POS_TYPE_FLOAT);
    rAssert(bird->spriteType == SPRITE_BIRD);
    rAssert(dt < 300);

    static f32 dy = 0.0f;

    if (dy < 0.0f)
        dy *= WORLD_UPDRAFT_DAMPENING;

    if (updraft) {
        dy = WORLD_UPDRAFT_V;
    } else {
        // v = a * t
        dy += WORLD_GRAVITY_ACCEL * (f32) dt / 1000;
    }

    // integrate vertical velocity to bird pos
    bird->pos.f.y += dy * (f32) dt / 1000;

    g_worldState = WORLD_NEED_REBUILD;
}