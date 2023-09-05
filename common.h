#ifndef __COMMON_H_
#define __COMMON_H_

#include <cstdio>
#include <cmath>
#include <cstring>
#include "raylib.h"
#include "raymath.h"

// what c primitive typenames should have been:
typedef unsigned char u8;
typedef char i8;
typedef unsigned short u16;
typedef short i16;
typedef unsigned int u32;
typedef int i32;
typedef unsigned long long int u64;
typedef long long int s64;
typedef float f32;
typedef double f64;
typedef bool b32;

// typedefing Raylib data types to make it more consistant with my coding style:
typedef Rectangle rect_t;
typedef Vector2 vector_t;
typedef Texture2D texture_t;
typedef Font font_t;

// a macro for the extremely annoying problem that a Raylib vector takes floats
// but we always use ints:
#define VECTOR(x, y) ((vector_t){(f32)x, (f32)y})
#define ZERO_VECTOR ((vector_t){0, 0})

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(n) ((n) > 0 ? (n) : -(n))

#endif