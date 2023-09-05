#include "common.h"
#include "raylib.h"

#define UI_MAX_ELEMENTS 128

enum animation_type_e : u32
{
    ANIMATION_NO_ANIMATION,
    ANIMATION_PLAY_ONCE,
    ANIMATION_LOOP,
};

enum mouse_actions : u32
{
    MOUSE_ACTION_NONE,
    MOUSE_ACTION_LMB_PRESSED,
    MOUSE_ACTION_RMB_PRESSED,
    MOUSE_ACTION_LMB_DOWN,
    MOUSE_ACTION_RMB_DOWN,
    MOUSE_ACTION_NUM_MOUSE_ACTIONS
};

enum axis_e : b32
{
    X_AXIS = false,
    Y_AXIS = true
};

struct ui_animation_t
{
    animation_type_e type;
    texture_t *texture;
    rect_t frame;
    axis_e layout_axis;
    u32 curr_frame;
    u32 num_frames;
    u32 frames_per_second;
    f32 timer;
};

enum ui_element_type_e : u32
{
    UI_ELEMENT_NO_ELEMENT,
    UI_ELEMENT_RECT,
    UI_ELEMENT_CIRCLE,
    NUM_UI_ELEMENT_TYPES
};

struct ui_element_t
{
    ui_element_type_e type;
    ui_animation_t animation;
    rect_t rect;
    b32 mouse_over;
    b32 mouse_lmb_pressed;
    b32 mouse_lmb_down;
    b32 mouse_rmb_pressed;
    b32 mouse_rmb_down;
    i32 (*mouse_actions[MOUSE_ACTION_NUM_MOUSE_ACTIONS])(void);
};

ui_element_t g_ui_elements[UI_MAX_ELEMENTS];

u32 ui_element_add(ui_element_t *element)
{
    for (u32 itr = 0; itr < UI_MAX_ELEMENTS; itr++)
    {
        ui_element_t *current = &g_ui_elements[itr];
        if (current->type == UI_ELEMENT_NO_ELEMENT)
        {
            *current = *element;
            return itr;
        }
    }
    return -1; // array full, no more elements can be added
}

u32 ui_element_remove(i32 id)
{
    g_ui_elements[id].type = UI_ELEMENT_NO_ELEMENT;
}

void ui_elements_update()
{
    for (u32 itr = 0; itr < UI_MAX_ELEMENTS; itr++)
    {
        ui_element_t *current = &g_ui_elements[itr];

        if (current->type != UI_ELEMENT_NO_ELEMENT)
        {
            // Update mouse state:
            current->mouse_over = CheckCollisionPointRec(GetMousePosition(), current->rect);
            current->mouse_lmb_pressed = current->mouse_over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            current->mouse_rmb_pressed = current->mouse_over && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
            current->mouse_lmb_down = current->mouse_over && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            current->mouse_rmb_down = current->mouse_over && IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

            // Fire callback funcs:
            if (current->mouse_lmb_pressed)
            {
                if (current->mouse_actions[MOUSE_ACTION_LMB_PRESSED] != 0)
                {
                    current->mouse_actions[MOUSE_ACTION_LMB_PRESSED]();
                }
            }

            if (current->mouse_rmb_pressed)
            {
                if (current->mouse_actions[MOUSE_ACTION_RMB_PRESSED] != 0)
                {
                    current->mouse_actions[MOUSE_ACTION_RMB_PRESSED]();
                }
            }

            if (current->mouse_lmb_down)
            {
                if (current->mouse_actions[MOUSE_ACTION_LMB_DOWN] != 0)
                {
                    current->mouse_actions[MOUSE_ACTION_LMB_DOWN]();
                }
            }

            if (current->mouse_rmb_down)
            {
                if (current->mouse_actions[MOUSE_ACTION_RMB_DOWN] != 0)
                {
                    current->mouse_actions[MOUSE_ACTION_RMB_DOWN]();
                }
            }

            // Update animation:
            ui_animation_t *animation = &current->animation;
            switch (animation->type)
            {
            case ANIMATION_NO_ANIMATION:
                break;
            case ANIMATION_PLAY_ONCE:
            {
                animation->timer += GetFrameTime();
                if (animation->timer < (1.0f / animation->frames_per_second))
                {
                    if (animation->curr_frame != animation->num_frames)
                    {
                        animation->curr_frame++;
                    }
                    animation->timer = 0.0f;
                }
            }
            break;
            case ANIMATION_LOOP:
            {
                animation->timer += GetFrameTime();
                if (animation->timer < (1.0f / animation->frames_per_second))
                {
                    animation->curr_frame++;
                    animation->curr_frame %= animation->num_frames;
                    animation->timer = 0.0f;
                }
            }
            break;
            }
        }
    }
}

void ui_elements_render()
{
    for (u32 itr = 0; itr < UI_MAX_ELEMENTS; itr++)
    {
        ui_element_t *current = &g_ui_elements[itr];
        if (current->type != UI_ELEMENT_NO_ELEMENT)
        {
            // Render animation:
            ui_animation_t *animation = &current->animation;
            if (animation->type != ANIMATION_NO_ANIMATION)
            {
                Rectangle src = animation->frame;
                src.x += animation->layout_axis == X_AXIS ? animation->frame.width * animation->curr_frame : 0;
                src.y += animation->layout_axis == Y_AXIS ? animation->frame.height * animation->curr_frame : 0;
                DrawTexturePro(
                    *animation->texture,
                    src, current->rect,
                    (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
            }
        }
    }
}
