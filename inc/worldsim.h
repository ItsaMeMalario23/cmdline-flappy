#ifndef WORLDSIM_H
#define WORLDSIM_H

#include <main.h>

// wstate values
#define WORLD_UNINIT        0
#define WORLD_NOP           1
#define WORLD_NEED_REBUILD  2

#define WORLD_MAX_SPRITES   8

// position vector types
#define POS_TYPE_FLOAT      0
#define POS_TYPE_INT        1

// sprite types
#define SPRITE_UNDEF        0
#define SPRITE_BIRD         1
#define SPRITE_PIPE         2

// world simulation default valules
#define WORLD_STD_GRAVITY_DV        ( 10.0f)
#define WORLD_STD_UPDRAFT_V         (-27.0f)
#define WORLD_STD_UPDRAFT_DAMPING   (  0.9f)
#define WORLD_STD_SCROLL_SPEED      ( 1)
#define WORLD_STD_BIRD_XPOS         ( 23.0f)
#define WORLD_STD_BIRD_YPOS         ( 10.0f)
#define WORLD_STD_FIRST_PIPE        ( 47)
#define WORLD_STD_PIPE_DISTANCE     ( 50)

// typedefs
typedef u8 wstate_t;        // type for world states
typedef u8 texturedata_t;   // type for sprite textures

typedef struct vec3f_s {    // floating point vector
    f32 x;
    f32 y;
} vec3f_t;

typedef struct vec3i_t {    // integer vector
    i32 x;
    i32 y;
} vec3i_t;

typedef struct sprite_s {   // sprite data
    u8* data;
    u16 width;
    u16 height;
    u16 posType;
    u16 spriteType;
    
    union {
        vec3f_t f;
        vec3i_t i;
    } pos;

} sprite_t;

sprite_t* addSprite(u16 width, u16 height, u16 posType, u16 spriteType, i32 xpos_i, i32 ypos_i, f32 xpos_f, f32 ypos_f);
sprite_t* addPipe(i32 xpos, i32 ypos, u16 height, bool direction);
sprite_t* addBird(f32 xpos, f32 ypos);

void moveSprite(sprite_t* sprite, i32 dx_i, i32 dy_i, f32 dx_f, f32 dy_f);
void moveSpriteI(sprite_t* sprite, i32 dx, i32 dy);
void moveSpriteF(sprite_t* sprite, f32 dx, f32 dy);
void renderSprite(sprite_t* sprite);

void initWorld(void);
void cleanupWorld(void);
void requestWorldRebuild(void);
i64  updateWorld(u64 dt, bool updraft);

sprite_t* getBird(void);
void scrollScreen(void);
void resetPipe(sprite_t* pipe);
i32  randomizePipe(bool direction);
bool checkCollision(sprite_t* bird);
void handleBirdVerticalSpeed(sprite_t* bird, u64 dt, bool updraft);

#endif