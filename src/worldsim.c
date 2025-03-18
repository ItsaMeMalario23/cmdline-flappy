#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <worldsim.h>
#include <render.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

wstate_t g_worldState = WORLD_UNINIT;

texturedata_t g_worldRenderBuf[WORLD_HEIGHT][WORLD_WIDTH];

sprite_t g_wSprites[WORLD_MAX_SPRITES];
u8 g_wSpriteIdx = 0;

i64 g_wGameScore = 0;

// world simulation settings
f32 g_worldGravityAccel =       WORLD_STD_GRAVITY_DV;
f32 g_worldUpdraftVelocity =    WORLD_STD_UPDRAFT_V;
f32 g_worldUpdraftDamping =     WORLD_STD_UPDRAFT_DAMPING;
f32 g_worldBirdXPos =           WORLD_STD_BIRD_XPOS;
f32 g_worldBirdYPos =           WORLD_STD_BIRD_YPOS;
u16 g_worldFirstPipeDistance =  WORLD_STD_FIRST_PIPE;
u16 g_worldPipeDistance =       WORLD_STD_PIPE_DISTANCE;
u8  g_worldScrollInterval =     WORLD_STD_SCROLL_SPEED;

// sprite textures
const texturedata_t g_pipeData[4][8] = {
    { 2,  2,  2,  2,  2,  2,  2,  2},
    { 2, 10, 10, 10, 15, 10, 10,  2},
    {16,  2, 10, 10, 15, 10,  2, 16},
    {16,  2, 10, 10, 15, 10,  2, 16}
};

const texturedata_t g_pipeDataY[4][8] = {
    { 3,  3,  3,  3,  3,  3,  3,  3},
    { 3, 11, 11, 11, 15, 11, 11,  3},
    {16,  3, 11, 11, 15, 11,  3, 16},
    {16,  3, 11, 11, 15, 11,  3, 16}
};

const texturedata_t g_birdData[40] = {
    16, 16,  3,  3,  3,  3,  3, 16,
    16,  3,  3, 15,  0,  3,  0, 16,
     3,  3,  3,  3,  3,  1,  1,  3,
    16,  3,  3,  3,  1,  1,  1,  1,
    16, 16,  3,  3,  3,  3,  3, 16,
};

static inline i32 randRange(i32 lbound, i32 ubound)
{
    rAssert(ubound > lbound);

    return (rand() % (ubound - lbound)) + lbound;
}

// getters
f32 getGravity(void) { return g_worldGravityAccel; }

f32 getUpdraftStrength(void) { return -g_worldUpdraftVelocity; }

f32 getUpdraftDamping(void) { return g_worldUpdraftDamping; }

u8  getScrollInterval(void) { return g_worldScrollInterval; }

f32 getBirdXPos(void) { return g_worldBirdXPos; }

f32 getBirdYPos(void) { return g_worldBirdYPos; }

u16 getFirstPipeDistance(void) { return g_worldFirstPipeDistance; }

u16 getPipeDistance(void) { return g_worldPipeDistance; }

// parameter so function can be called directly from menu nav
void setDefaults(u8 context)
{
    g_worldGravityAccel = WORLD_STD_GRAVITY_DV;
    g_worldUpdraftVelocity = WORLD_STD_UPDRAFT_V;
    g_worldUpdraftDamping = WORLD_STD_UPDRAFT_DAMPING;
    g_worldBirdXPos = WORLD_STD_BIRD_XPOS;
    g_worldBirdYPos = WORLD_STD_BIRD_YPOS;
    g_worldFirstPipeDistance = WORLD_STD_FIRST_PIPE;
    g_worldPipeDistance = WORLD_STD_PIPE_DISTANCE;
    g_worldScrollInterval = WORLD_STD_SCROLL_SPEED;
}

void incrementGravity(u8 context)
{
    if (context == SETTINGS_INCREASE && g_worldGravityAccel + SETTINGS_GRAVITY_INCR < WORLD_MAX_GRAVITY)
        g_worldGravityAccel += SETTINGS_GRAVITY_INCR;
    else if (context == SETTINGS_DECREASE && g_worldGravityAccel - SETTINGS_GRAVITY_INCR > 0.0f)
        g_worldGravityAccel -= SETTINGS_GRAVITY_INCR;
}

void incrementUpdraftStrength(u8 context)
{
    if (context == SETTINGS_DECREASE && g_worldUpdraftVelocity - SETTINGS_UPDRAFT_INCR < 0.0f)
        g_worldUpdraftVelocity -= SETTINGS_UPDRAFT_INCR;
    else if (context == SETTINGS_INCREASE && g_worldUpdraftVelocity + SETTINGS_UPDRAFT_INCR > WORLD_MAX_UPDRAFT_V)
        g_worldUpdraftVelocity += SETTINGS_UPDRAFT_INCR;
}

void incrementUpdraftDamping(u8 context)
{
    if (context == SETTINGS_INCREASE && g_worldUpdraftDamping + SETTINGS_DAMPING_INCR < WORLD_MAX_UPDRAFT_DAMPING)
        g_worldUpdraftDamping += SETTINGS_DAMPING_INCR;
    else if (context == SETTINGS_DECREASE && g_worldUpdraftDamping - SETTINGS_DAMPING_INCR > WORLD_MIN_UPDRAFT_DAMPING)
        g_worldUpdraftDamping -= SETTINGS_DAMPING_INCR;
}

void incrementScrollInterval(u8 context)
{
    if (context == SETTINGS_INCREASE)
        g_worldScrollInterval++;
    else if (context == SETTINGS_DECREASE && g_worldScrollInterval > 1)
        g_worldScrollInterval--;
}

void incrementBirdXPos(u8 context)
{
    if (context == SETTINGS_INCREASE && g_worldBirdXPos + SETTINGS_POS_INCR < WORLD_MAX_BIRD_XPOS)
        g_worldBirdXPos += SETTINGS_POS_INCR;
    else if (context == SETTINGS_DECREASE && g_worldBirdXPos - SETTINGS_POS_INCR > 0.0f)
        g_worldBirdXPos -= SETTINGS_POS_INCR;
}

void incrementBirdYPos(u8 context)
{
    if (context == SETTINGS_INCREASE && g_worldBirdYPos + SETTINGS_POS_INCR < WORLD_MAX_BIRD_YPOS)
        g_worldBirdYPos += SETTINGS_POS_INCR;
    else if (context == SETTINGS_DECREASE && g_worldBirdYPos - SETTINGS_POS_INCR > 0.0f)
        g_worldBirdYPos -= SETTINGS_POS_INCR;
}

void incrementFirstPipe(u8 context)
{
    if (context == SETTINGS_INCREASE && g_worldFirstPipeDistance + SETTINGS_POS_INCR < WORLD_MAX_FIRST_PIPE)
        g_worldFirstPipeDistance += SETTINGS_POS_INCR;
    else if (context == SETTINGS_DECREASE && g_worldFirstPipeDistance - SETTINGS_POS_INCR > 0)
        g_worldFirstPipeDistance -= SETTINGS_POS_INCR;
}

void incrementPipeDistance(u8 context)
{
    if (context == SETTINGS_INCREASE && g_worldPipeDistance + SETTINGS_POS_INCR < WORLD_MAX_PIPE_DISTANCE)
        g_worldPipeDistance += SETTINGS_POS_INCR;
    else if (context == SETTINGS_DECREASE && g_worldPipeDistance - SETTINGS_POS_INCR > 0)
        g_worldPipeDistance -= SETTINGS_POS_INCR;
}

sprite_t* addSprite(u16 width, u16 height, u16 posType, u16 spriteType, i32 xpos_i, i32 ypos_i, f32 xpos_f, f32 ypos_f)
{
    if (g_wSpriteIdx > WORLD_MAX_SPRITES - 1)
        return NULL;

    rAssert(width);
    rAssert(height);
    rAssert(posType == POS_TYPE_FLOAT || posType == POS_TYPE_INT);

    sprite_t* sprite = g_wSprites + g_wSpriteIdx;

    g_wSpriteIdx++;

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

sprite_t* addPipe(i32 xpos, i32 ypos, u16 height, bool direction)
{
    sprite_t* sprite = addSprite(8, height + 4, POS_TYPE_INT, SPRITE_PIPE, xpos, ypos, 0.0f, 0.0f);

    if (!sprite)
        return NULL;

    if (direction) {
        memcpy(sprite->data, g_pipeData[0], sizeof(u8) * 8);
        memcpy(sprite->data + 8, g_pipeData[1], sizeof(u8) * 8);
        memcpy(sprite->data + 16, g_pipeData[1], sizeof(u8) * 8);
        memcpy(sprite->data + 24, g_pipeData[2], sizeof(u8) * 8);
    } else {
        memcpy(sprite->data + (height + 3) * 8, g_pipeData[0], sizeof(u8) * 8);
        memcpy(sprite->data + (height + 2) * 8, g_pipeData[1], sizeof(u8) * 8);
        memcpy(sprite->data + (height + 1) * 8, g_pipeData[1], sizeof(u8) * 8);
        memcpy(sprite->data + (height * 8), g_pipeData[2], sizeof(u8) * 8);
    }

    u8 offset = direction ? 24 : 0;

    for (u8 i = 0; i < height; i++)
        memcpy(sprite->data + offset + i * 8, g_pipeData[3], sizeof(u8) * 8);

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
            if (*src == PXID_CLEAR || xpos + x < 0) {
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
    memset(g_wSprites, 0, sizeof(g_wSprites));
    memset(g_worldRenderBuf, PXID_WHITE, sizeof(g_worldRenderBuf));

    (void) addBird(g_worldBirdXPos, g_worldBirdYPos);

    (void) addPipe(g_worldBirdXPos + g_worldFirstPipeDistance, randomizePipe(1), 20, 1);
    (void) addPipe(g_worldBirdXPos + g_worldFirstPipeDistance, randomizePipe(0), 20, 0);
    (void) addPipe(g_worldBirdXPos + g_worldFirstPipeDistance + g_worldPipeDistance, randomizePipe(1), 20, 1);
    (void) addPipe(g_worldBirdXPos + g_worldFirstPipeDistance + g_worldPipeDistance, randomizePipe(0), 20, 0);
    (void) addPipe(g_worldBirdXPos + g_worldFirstPipeDistance + (g_worldPipeDistance * 2), randomizePipe(1), 20, 1);
    (void) addPipe(g_worldBirdXPos + g_worldFirstPipeDistance + (g_worldPipeDistance * 2), randomizePipe(0), 20, 0);

    fflush(stdin);

    srand(time(NULL));

    g_wGameScore = 0;
    g_worldState = WORLD_NEED_REBUILD;
}

void cleanupWorld(void)
{
    for (u8 i = 0; i < g_wSpriteIdx; i++)
        memFree(g_wSprites[i].data);

    memset(g_wSprites, 0, sizeof(g_wSprites));

    g_wSpriteIdx = 0;
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

    static u8 scrollTimer = 0;
    static sprite_t* bird = NULL;

    if (!bird)
        bird = getBird();

    rAssert(bird);

    // scroll screen
    if (++scrollTimer >= g_worldScrollInterval) {
        scrollScreen();
        scrollTimer = 0;
    }

    // move bird
    handleBirdVerticalSpeed(bird, dt, updraft);

    // check bird collision
    if (checkCollision(bird))
        return g_wGameScore;

    // update score
    g_wGameScore += 100;

    // render world
    memset(g_worldRenderBuf, PXID_WHITE, sizeof(g_worldRenderBuf));

    for (u8 i = 0; i < g_wSpriteIdx; i++)
        renderSprite(&g_wSprites[i]);

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
    for (u8 i = 0; i < g_wSpriteIdx; i++) {
        if (g_wSprites[i].spriteType == SPRITE_BIRD)
            return g_wSprites + i;
    }

    return NULL;
}

// move all pipe sprites left by 1
void scrollScreen(void)
{
    sprite_t* tmp;

    for (u8 i = 0; i < g_wSpriteIdx; i++) {
        if (g_wSprites[i].spriteType != SPRITE_PIPE)
            continue;

        tmp = g_wSprites + i;

        rAssert(tmp->posType == POS_TYPE_INT);

        if (tmp->pos.i.x < -60)
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

    if (pipe->pos.i.y > WORLD_HEIGHT / 2)
        pipe->pos.i.y = randomizePipe(1);
    else
        pipe->pos.i.y = randomizePipe(0);
}

// check if pipe is top or bottom and randomize position
i32 randomizePipe(bool direction)
{
    if (direction)
        return randRange((WORLD_HEIGHT / 2) + 1, WORLD_HEIGHT - 4);
    else
        return randRange(-20, -12);
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
    rAssert(bird->pos.f.x + 0.01f > g_worldBirdXPos && bird->pos.f.x - 0.01f < g_worldBirdXPos);

    sprite_t* tmp;

    for (u8 i = 0; i < g_wSpriteIdx; i++) {
        tmp = g_wSprites + i;

        if (tmp->spriteType != SPRITE_PIPE)
            continue;

        rAssert(tmp->posType == POS_TYPE_INT);

        // check for horizontal collision
        if (tmp->pos.i.x > g_worldBirdXPos + bird->width || tmp->pos.i.x + tmp->width < g_worldBirdXPos)
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
        dy *= g_worldUpdraftDamping;

    if (updraft) {
        dy = g_worldUpdraftVelocity;
    } else {
        // v = a * t
        dy += g_worldGravityAccel * (f32) dt / 1000;
    }

    // integrate vertical velocity to bird pos
    bird->pos.f.y += dy * (f32) dt / 1000;

    g_worldState = WORLD_NEED_REBUILD;
}