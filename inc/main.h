#ifndef MAIN_H
#define MAIN_H

#define R_DEBUG
#define RDEBUG_BREAK_EXIT

#define SCREEN_WIDTH        160
#define SCREEN_HEIGHT       48

#define SCR_BUF_SIZE        32768
#define SCR_ERR_BUF_SIZE    64

#define SCR_EMPTY           0
#define SCR_INCOMPLETE      1
#define SCR_ERROR           2
#define SCR_READY           3

#define WORLD_HEIGHT        ((SCREEN_HEIGHT - 2))
#define WORLD_WIDTH         (((SCREEN_WIDTH - 2) / 2))

#define FIXED_FRAME_TIME    36

#define EPSILON             (0.00000001f)

// Typedefs
typedef signed char i8;
typedef unsigned char u8;

typedef signed short int i16;
typedef unsigned short int u16;

typedef signed long int i32;
typedef unsigned long int u32;

typedef signed long long i64;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef unsigned char bool;

// Function types
typedef void (*nav_f)(u8);

// Structs
typedef struct menu_s {
    const char*   title;
    const char**  items;
    const nav_f*  navcom;
    u32           navkey;
    u32           len;
} menu_t;

#endif