// PARAPPA THE WRAPPER.H
// This file has a silly filename to remind us that
// we might one day be replacing raylib with something else
// and when that day comes, all we have to do is to change
// these functions and types to our own and replace these
// proxy functions with our own thing (or Simp if we move to JAI)

#ifndef _KICK_PUNCH_IT_S_ALL_IN_THE_MIND_
#define _KICK_PUNCH_IT_S_ALL_IN_THE_MIND_

#include "raylib.h"
#include "raymath.h"

typedef Vector2 vector_t;
typedef Rectangle rect_t;
typedef Color color_t;
typedef Texture2D texture_t;
typedef Font font_t;

void init_window(int width, int height)
{
    InitWindow(width, height, "Sphere City 7 Combat Demo");
}

bool window_should_close()
{
    return WindowShouldClose();
}

void close_window()
{
    CloseWindow();
}

void clear_background(color_t color)
{
    ClearBackground(color);
}

void begin_drawing()
{
    BeginDrawing();
}

void end_drawing()
{
    EndDrawing();
}

texture_t load_texture(char *path)
{
    return LoadTexture(path);
}

void draw_texture(texture_t texture, rect_t src, rect_t dest,
                  vector_t origin = (vector_t){0.0f, 0.0f},
                  float rotation = 0.0f, color_t color = WHITE)
{
    DrawTexturePro(texture, src, dest, origin, rotation, color);
}

void unload_texture(texture_t texture)
{
    UnloadTexture(texture);
}

void draw_rectangle(int x, int y, int w, int h, color_t color)
{
    DrawRectangle(x, y, w, h, color);
}

void draw_rectangle(rect_t rec, color_t color)
{
    DrawRectangleRec(rec, color);
}

void draw_rectangle_lines(int x, int y, int w, int h, color_t color)
{
    DrawRectangleLines(x, y, w, h, color);
}

void draw_text(font_t font, const char *text, vector_t pos,
               vector_t origin = (vector_t){0.0f, 0.0f}, float rotation = 0.0f,
               int font_size = 16, int spacing = 16, color_t tint = WHITE)
{
    DrawTextPro(font, text, pos, origin, rotation, font_size, spacing, tint);
}

#endif