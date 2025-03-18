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

// settings operation types
#define SETTINGS_DECREASE   0
#define SETTINGS_INCREASE   1
#define SETTINGS_NOP        2

// world simulation default values
#define WORLD_STD_GRAVITY_DV        ( 10.0f)
#define WORLD_STD_UPDRAFT_V         (-27.0f)
#define WORLD_STD_UPDRAFT_DAMPING   (  0.9f)
#define WORLD_STD_SCROLL_SPEED      (  1)
#define WORLD_STD_BIRD_XPOS         ( 23.0f)
#define WORLD_STD_BIRD_YPOS         ( 10.0f)
#define WORLD_STD_FIRST_PIPE        ( 47)
#define WORLD_STD_PIPE_DISTANCE     ( 50)

#define WORLD_MAX_GRAVITY           ( 100.0f)
#define WORLD_MAX_UPDRAFT_V         (-200.0f)
#define WORLD_MIN_UPDRAFT_DAMPING   (   0.1f)
#define WORLD_MAX_UPDRAFT_DAMPING   (   1.0f)
#define WORLD_MAX_BIRD_XPOS         (WORLD_WIDTH - 8)
#define WORLD_MAX_BIRD_YPOS         (WORLD_HEIGHT - 6)
#define WORLD_MAX_FIRST_PIPE        ( 200)
#define WORLD_MAX_PIPE_DISTANCE     ( 200)

#define SETTINGS_GRAVITY_INCR       ( 1.0f)
#define SETTINGS_UPDRAFT_INCR       (-1.0f)
#define SETTINGS_DAMPING_INCR       ( 0.01f)
#define SETTINGS_SCROLL_INCR        ( 1)
#define SETTINGS_POS_INCR           ( 1.0f)

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

//
//  Public functions
//
f32 getGravity(void);
f32 getUpdraftStrength(void);
f32 getUpdraftDamping(void);
u8  getScrollInterval(void);
f32 getBirdXPos(void);
f32 getBirdYPos(void);
u16 getFirstPipeDistance(void);
u16 getPipeDistance(void);

void setDefaults(u8 context);
void incrementGravity(u8 context);
void incrementUpdraftStrength(u8 context);
void incrementUpdraftDamping(u8 context);
void incrementScrollInterval(u8 context);
void incrementBirdXPos(u8 context);
void incrementBirdYPos(u8 context);
void incrementFirstPipe(u8 context);
void incrementPipeDistance(u8 context);

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

//
//  Local functions
//
sprite_t* getBird(void);
void scrollScreen(void);
void resetPipe(sprite_t* pipe);
i32  randomizePipe(bool direction);
bool checkCollision(sprite_t* bird);
void handleBirdVerticalSpeed(sprite_t* bird, u64 dt, bool updraft);

#endif