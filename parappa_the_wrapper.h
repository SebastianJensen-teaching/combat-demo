// PARAPPA THE WRAPPER.H
// This file has a silly filename to remind us that
// we might one day be replacing raylib with something else
// and when that day comes, all we have to do is to change
// these functions and types to our own and replace these
// proxy functions with our own thing (or Simp if we move to JAI)

#ifndef _KICK_PUNCH_IT_S_ALL_IN_THE_MIND_
#define _KICK_PUNCH_IT_S_ALL_IN_THE_MIND_

#define USE_RAYLIB
// #define PLATFORM_WIN32
// #define PLATFORM_OSX
// #define PLATFORM_LINUX

#ifdef PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan.h>
#elifdef PLATFORM_LINUX
#include <xcb/xcb.h>
#include <vulkan/vulkan.h>
#elifdef PLATFORM_OSX
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#elifdef USE_RAYLIB
#include "raylib.h"
#include "raymath.h"
typedef Vector2 vector_t;
typedef Rectangle rect_t;
typedef Color color_t;
typedef Texture2D texture_t;
typedef Font font_t;
#endif



void init_window(int width, int height)
{
#ifdef USE_RAYLIB
    InitWindow(width, height, "Sphere City 7 Combat Demo");
    ToggleFullscreen();
    SetTargetFPS(60);
    SetExitKey(KEY_X);
#endif
}

bool window_should_close()
{
#ifdef USE_RAYLIB
    return WindowShouldClose();
#endif
}

void close_window()
{
#ifdef USE_RAYLIB
    CloseWindow();
#endif
}

void clear_background(color_t color)
{
#ifdef USE_RAYLIB
    ClearBackground(color);
#endif
}

void begin_drawing()
{
#ifdef USE_RAYLIB
    BeginDrawing();
#endif
}

void end_drawing()
{
#ifdef USE_RAYLIB
    EndDrawing();
#endif
}

texture_t load_texture(char *path)
{
#ifdef USE_RAYLIB
    return LoadTexture(path);
#endif
}

void draw_texture(texture_t texture, rect_t src, rect_t dest,
                  vector_t origin = (vector_t){0.0f, 0.0f},
                  float rotation = 0.0f, color_t color = WHITE)
{
#ifdef USE_RAYLIB
    DrawTexturePro(texture, src, dest, origin, rotation, color);
#endif
}

void unload_texture(texture_t texture)
{
#ifdef USE_RAYLIB
    UnloadTexture(texture);
#endif
}

void draw_rectangle(int x, int y, int w, int h, color_t color)
{
#ifdef USE_RAYLIB
    DrawRectangle(x, y, w, h, color);
#endif
}

void draw_rectangle(rect_t rec, color_t color)
{
#ifdef USE_RAYLIB
    DrawRectangleRec(rec, color);
#endif
}

void draw_rectangle_lines(int x, int y, int w, int h, color_t color)
{
#ifdef USE_RAYLIB
    DrawRectangleLines(x, y, w, h, color);
#endif
}

inline void draw_text(font_t font, const char *text, vector_t pos,
                      vector_t origin = (vector_t){0.0f, 0.0f}, float rotation = 0.0f,
                      int font_size = 16, int spacing = 0, color_t tint = WHITE)
{
#ifdef USE_RAYLIB
    DrawTextPro(font, text, pos, origin, rotation, font_size, spacing, tint);
#endif
}

void set_random_seed(u32 seed)
{
#ifdef USE_RAYLIB
    SetRandomSeed(seed);
#endif
}

i32 get_random_int(i32 min, i32 max)
{
#ifdef USE_RAYLIB
    return GetRandomValue(min, max);
#endif
}

#endif