#include "common.h"

// @TODO: Word wrap! There must be really good known solutions to that ...
void ui_text_box(const char *text, rect_t rect, i32 font_size, Color color)
{
    i32 text_width = MeasureText(text, font_size);
    i32 num_lines = 1;
    if (text_width > rect.width)
    {
        num_lines = (i32)(ceilf(text_width / rect.width));
    }
    i32 chara_width = MeasureText("m", font_size);
    i32 chara_per_line = (i32)(ceilf(rect.width / chara_width));
    i32 line_height = font_size * 1.2f;
    i32 x_pos = rect.x;
    i32 y_pos = rect.y;
    i32 text_pos = 0;
    for (i32 line = 0; line < num_lines; line++)
    {
        draw_text(
            ui_font,
            TextSubtext(text, text_pos, chara_per_line),
            VECTOR(x_pos, y_pos),
            VECTOR(0, 0),
            0.0f, font_size, 0, color);
        text_pos += chara_per_line;
        y_pos += line_height;
    }
}

bool ui_end_turn_button()
{
    i32 cards_left = 0;
    i32 total_command_points = 0;
    for (int itr = 0; itr < COMBAT_HAND_SIZE; itr++)
    {
        if (g_cards_hand[itr] != 0)
        {
            cards_left++;
            total_command_points += g_card_db[g_cards_hand[itr]].value;
        }
    }

    const char *endturn_subtext = TextFormat(
        "Burn %d cards for %d CP", cards_left, total_command_points);

    vector_t endturn_text_size = MeasureTextEx(ui_font, "END TURN", 32, 4);
    vector_t endturn_subtext_size = MeasureTextEx(ui_font, endturn_subtext, 16, 0);

    static Rectangle button_rect{
        .x = game_width - (endturn_subtext_size.x + 32),
        .y = 16,
        .width = (endturn_subtext_size.x + 16),
        .height = 96};

    b32 mouse_over = CheckCollisionPointRec(g_virtual_mouse, button_rect);

    DrawRectangleRec(button_rect, mouse_over ? LIGHTGRAY : DARKGRAY);
    DrawTextEx(ui_font, "END TURN",
               (Vector2){button_rect.x + (button_rect.width / 2) - (endturn_text_size.x / 2),
                         button_rect.y + 8},
               32, 4, mouse_over ? BLACK : WHITE);
    if (cards_left)
    {
        DrawTextEx(ui_font, endturn_subtext,
                   (Vector2){button_rect.x + (button_rect.width / 2) - (endturn_subtext_size.x / 2),
                             button_rect.y + button_rect.height - endturn_subtext_size.y - 8},
                   16, 0, mouse_over ? BLACK : WHITE);
    }
    if (mouse_over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        return true;
    }
    return false;
}

void ui_command_indicator()
{
    rect_t src = {96.0f + (combat.command_points * 64.0f), 160, 64, 64};
    rect_t dest = {game_width - 328, 1, 128, 128};
    draw_texture(g_ui_texture, src, dest, ZERO_VECTOR, 0.0f, WHITE);
}

void ui_discard_pile()
{
    for (u32 itr = 0; itr < g_cards_discard_num; itr++)
    {
        card_t *card = &g_card_db[g_cards_discard[itr]];
        draw_text(ui_font, TextFormat("%2d: %s", itr, card->name),
                  VECTOR(GetScreenWidth() - 256, 128 + itr * 24),
                  ZERO_VECTOR, 0.0f, 16, 0,
                  itr == g_cards_discard_num ? YELLOW : WHITE);
    }
}

void ui_command_cards()
{

    rect_t source = {0, 0, 96, 128};
    rect_t dest = {(f32)COMBAT_UI_CARD_OFFSET_X, (f32)COMBAT_UI_CARD_OFFSET_Y, COMBAT_UI_CARD_WIDTH, COMBAT_UI_CARD_HEIGHT};

    for (int itr = 0; itr < COMBAT_HAND_SIZE; itr++)
    {
        if (g_cards_hand[itr] != 0)
        {
            i32 card_id = g_cards_hand[itr];
            Color card_color = class_colors[g_card_db[card_id].unit_class];
            draw_texture(g_ui_texture, source, dest, ZERO_VECTOR, 0.0f,
                         itr == combat.selected_card ? WHITE : itr == combat.mouse_card ? LIGHTGRAY
                                                                                        : GRAY);
            draw_texture(g_ui_texture,
                         (rect_t){source.x, source.y + 128 + 32, source.width, source.height},
                         dest, ZERO_VECTOR, 0.0f,
                         itr == combat.selected_card ? card_color : itr == combat.mouse_card ? LIGHTGRAY
                                                                                             : GRAY);
            draw_text(ui_font, TextFormat("[%d]%s", g_card_db[card_id].cost, g_card_db[card_id].name),
                      VECTOR(dest.x + 24, dest.y + 20), ZERO_VECTOR, 0.0f, 16, 0, card_color);
            rect_t description_text_rect = {dest.x + 32, dest.y + 96, 120, 120};
            ui_text_box(g_card_db[card_id].description, description_text_rect, 16, RAYWHITE);

            {
                rect_t value_src = {
                    .x = 96 + g_card_db[card_id].value * 64.0f,
                    .y = 160,
                    .width = 64,
                    .height = 64};
                rect_t value_dest = {
                    .x = dest.x + 148,
                    .y = dest.y + 12,
                    .width = 32,
                    .height = 32};
                draw_texture(g_ui_texture, value_src, value_dest, ZERO_VECTOR, 0.0f, WHITE);
            }

            // Todo: Move this whole thing out of the loop, we only need to do it once
            // CARD ACTION BUTTONS
            {
                if (combat.selected_card == itr)
                {
                    combat.button_timer += GetFrameTime();
                    if (combat.button_timer > 0.05f)
                    {
                        if (combat.button_frame < 3)
                        {
                            combat.button_frame++;
                        }
                        combat.button_timer = 0.0f;
                    }
                    rect_t buttons_src = {
                        .x = 97,
                        .y = combat.button_frame * 32.0f,
                        .width = 96,
                        .height = 32};
                    rect_t buttons_dest = {
                        dest.x, dest.y - 64, dest.width, 64};
                    draw_texture(g_ui_texture, buttons_src, buttons_dest, ZERO_VECTOR, 0.0f, WHITE);
                }
            }
        }
        else
        {
            draw_rectangle(dest, GRAY);
        }
        dest.x += COMBAT_UI_CARD_WIDTH;
    }
}

bool ui_pc_card(i32 id, i32 x, i32 y)
{
    player_character_t *chara = &g_player_characters[id];
    Color bg_color = class_colors[chara->unit_class];
    // Calibrate color brightness crudely based on state:
    {
        static float brightness[] = {
            0.0f,  // empty
            0.0f,  // dead
            0.75f, // ready
            1.0f,  // active
            0.30f, // exhausted
            1.0f   // reactivated
        };
        bg_color.r *= brightness[chara->state];
        bg_color.g *= brightness[chara->state];
        bg_color.b *= brightness[chara->state];
    }
    // bg_color.a = activation_alpha_values[chara->state];
    draw_texture(g_ui_texture, (rect_t){320.0f, 0.0f, 128.0f, 64.0f}, (rect_t){(f32)x, (f32)y, 256.0f, 128.0f}, ZERO_VECTOR, 0.0f, bg_color);
    DrawTextPro(ui_font, TextFormat("%s", chara->name), VECTOR(x + 128.0f, y + 6.0f), ZERO_VECTOR, 0.0f, 24, 0, bg_color);
    return false;
}

i32 ui_pc_cards(i32 x, i32 y)
{
    static i32 card_height = 128;
    static i32 selected = -1;
    for (int i = 0; i < sizeof(g_player_characters) / sizeof(player_character_t); i++)
    {
        if (ui_pc_card(i, x, y))
        {
            selected = i;
        }
        y += card_height + 8;
    }
    return selected;
}

void combat_ui_render()
{
    ui_pc_cards(0, 88);
    ui_command_cards();
    ui_discard_pile();
    if (ui_end_turn_button())
    {
        turn_end();
    }
    ui_command_indicator();
    draw_text(ui_font, TextFormat("Turn: %d\tCards remaing: %d", combat.current_turn, g_deck_top),
              (vector_t){8.0f, 8.0f});
}