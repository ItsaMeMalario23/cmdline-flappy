#ifndef WORLDSIM_H
#define WORLDSIM_H

#include <main.h>

#define WORLD_UNINIT        0
#define WORLD_NOP           1
#define WORLD_NEED_REBUILD  2

#define WORLD_MAX_SPRITES   8

sprite_t* addSprite(u16 width, u16 height, u16 xpos, u16 ypos);
sprite_t* addPipe(u32 height, u16 xpos, u16 ypos);
sprite_t* addBird(u16 xpos, u16 ypos);

void moveSprite(sprite_t* sprite, i16 dx, i16 dy);
void renderSprite(sprite_t* sprite);

void initWorld(void);
void cleanupWorld(void);
void requestWorldRebuild(void);
void updateWorld(void);

#endif