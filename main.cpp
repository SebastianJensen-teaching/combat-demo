#include "common.h"

#define ENABLE_CHECKS 1 // enables bounds checks and other debug checks

#define COMBAT_MAP_WIDTH 9
#define COMBAT_MAP_HEIGHT 9
#define COMBAT_MAP_OFFSET_X 256
#define COMBAT_MAP_OFFSET_Y 128

#define COMBAT_TILES_NUM (COMBAT_MAP_WIDTH * COMBAT_MAP_HEIGHT)
#define COMBAT_TILE_WIDTH 64
#define COMBAT_TILE_HEIGHT 64

#define COMBAT_DECK_SIZE 30
#define COMBAT_HAND_SIZE 10

#define COMBAT_UI_CARD_WIDTH 192
#define COMBAT_UI_CARD_HEIGHT 256
#define COMBAT_UI_CARD_OFFSET_X 0
#define COMBAT_UI_CARD_OFFSET_Y (1080 - COMBAT_UI_CARD_HEIGHT)

#define TILE_ID(x, y) (x >= 0 && x < COMBAT_MAP_WIDTH && y >= 0 && y < COMBAT_MAP_HEIGHT ? (y * COMBAT_MAP_WIDTH + x) : -1)

const u32 game_width = 1920;
const u32 game_height = 1080;

vector_t g_virtual_mouse;
texture_t g_ui_texture;
font_t ui_font;

#include "pathfinding.cpp"

// GAME STATE:

enum game_state_e : u8
{
    GAME_STATE_COMBAT
};

game_state_e g_game_state = GAME_STATE_COMBAT;

// COMBAT STATE:

enum combat_state_e
{
    COMBAT_STATE_PRE,
    COMBAT_STATE_PLAYER_TURN_PRE,
    COMBAT_STATE_PLAYER_MAIN,
    COMBAT_STATE_PLAYER_PROCESS_CARD,
    COMBAT_STATE_PLAYER_SELECTION,
    COMBAT_STATE_PLAYER_CONFIRM_SELECTION,
    COMBAT_STATE_PLAYER_ACTOR_ACTIVE,
    COMBAT_STATE_ENEMY_TURN_PRE,
    COMBAT_STATE_ENEMY_TURN,
    COMBAT_STATE_POST
};

enum selection_mode_e
{
    SELECTION_MODE_NO_SELECTION = 0,
    SELECTION_MODE_CARD = 1 << 0,
    SELECTION_MODE_ACTOR = 1 << 1,
    SELECTION_MODE_NPC = 4,
    SELECTION_MODE_TILE = 8
};

// Either we could grow this into a bigger structure and keep all of the combat related
// data in it, or we could collapse it out if the number of members remain small anyway.
// The pro of keeping things in here is just that the global namespace isnt as polluted,
// but it is a serious drag to have to write g_combat_state. in front of everything ...
// I guess keeping it together would also make it easier to pass it around if we decide
// it can no longer be global (why not?)
struct combat_state_t
{
    combat_state_e state;
    selection_mode_e selection_mode;
    bfs_result movement_overlay;
    i32 mouse_tile_x;
    i32 mouse_tile_y;
    i32 mouse_tile_id;
    i32 mouse_card;
    i32 selected_actor;
    i32 selected_actor_tile;
    i32 selected_card;
    i32 button_frame;
    f32 button_timer;
    u32 command_points;
    i32 processing_card;
    char prompt[64];
};

combat_state_t combat = {
    .state = COMBAT_STATE_PLAYER_MAIN,
    .selection_mode = SELECTION_MODE_ACTOR,
    .mouse_card = -1,
    .selected_actor = -1,
    .selected_card = -1};

// GAME DATA:

enum unit_class
{
    UNIT_CLASS_ASSEMBLER,
    UNIT_CLASS_SLAYER,
    UNIT_CLASS_MANOWAR,
    UNIT_CLASS_WIZARD
};

const char *unit_class_strings[] = {
    "ASSEMBLER",
    "SLAYER",
    "MANOWAR",
    "WIZARD"};

struct player_character_t
{
    char name[24];
    u8 unit_class;
    u8 brains;
    u8 brawn;
    u8 swift;
    u8 guts;
    u8 hit_points;
    u8 move_points;
};

player_character_t g_player_characters[] = {
    {.name = "Brigand\0",
     .unit_class = UNIT_CLASS_ASSEMBLER,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Gentleman\0",
     .unit_class = UNIT_CLASS_WIZARD,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Mercenary\0",
     .unit_class = UNIT_CLASS_MANOWAR,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Navvie\0",
     .unit_class = UNIT_CLASS_SLAYER,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Preacher\0",
     .unit_class = UNIT_CLASS_ASSEMBLER,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
};

// ACTORS:
#include "actor.cpp"

enum command_cards
{
    CARD_NO_CARD,
    CARD_STEALTH,
    CARD_HACK,
    CARD_DISARM,
    CARD_STEAL,
    CARD_EMBOLDEN,
    CARD_REINFORCE,
    CARD_HARDEN,
    CARD_DEADENED,
    CARD_BOLSTER,
    CARD_FRONT_LINE,
    CARD_ICEOLATE,
    CARD_SHUTDOWN,
    CARD_MANIACAL,
    CARD_NEOLOGIC_SPASM,
    CARD_RECOVER,
    CARD_ROW_SPASM,
    CARD_COLUMN_SPASM,
    CARD_REST_ASSURED,
    CARD_SIMPLE_PLAN,
    CARD_PROPECHY,
    CARD_CONSPIRACY,
    CARD_MINDPHASER,
    CARD_TACTICAL_NEURAL_IMPLANT,
    CARD_IMMUNITY,
    CARD_PAINKILLER,
    CARD_OVERKILL,
    CARD_UNSTOPPABLE,
    CARD_PURGE,
    CARD_CLEANSE,
    CARD_BARRIER,
    CARD_NUM_CARDS
};

struct card_t
{
    u8 cost;
    u8 value;
    u8 unit_class;
    const char *name;
    const char *description;
};

card_t g_card_db[] = {
    {.name = "NO_CARD"},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_ASSEMBLER,
     .name = "FIRST",
     .description = "ASSEMBLER remains hidden until next activated or discovered"},
    {.cost = 2,
     .value = 2,
     .unit_class = UNIT_CLASS_WIZARD,
     .name = "SECOND",
     .description = "ASSEMBLER attempts to hack adjacent TERMINAL"},
    {.cost = 3,
     .value = 3,
     .unit_class = UNIT_CLASS_SLAYER,
     .name = "THIRD",
     .description = "ASSEMBLER attempts to disarm adjacent TRAP"},
    {.cost = 4,
     .value = 4,
     .unit_class = UNIT_CLASS_MANOWAR,
     .name = "FOURTH",
     .description = "ASSEMBLER attempts to steal item from adjacent ENEMY"}};

u32 g_cards_deck[COMBAT_DECK_SIZE];
u32 g_cards_discard[COMBAT_DECK_SIZE];
u32 g_cards_discard_num;
i32 g_recent_discard_came_from;
i32 g_cards_hand[COMBAT_HAND_SIZE];

void hand_discard(i32 card)
{
    g_recent_discard_came_from = card;
    if (g_cards_discard_num < COMBAT_DECK_SIZE)
    {
        g_cards_discard[g_cards_discard_num] = g_cards_hand[card];
        g_cards_discard_num++;
    }
    g_cards_hand[card] = 0;
}

//@NOTE: ONLY returns the card to hand, not reimbursing the player for any lost
// cost or return points gained from card etc.
void undo_most_recent_discard()
{
    g_cards_discard_num--;
    g_cards_hand[g_recent_discard_came_from] = g_cards_discard[g_cards_discard_num];
    g_cards_discard[g_cards_discard_num] = 0;
}

// @TODO: Word wrap! There must be really good known solutions to that ...
// @TODO: For card descriptions we want to use an escape code system that allows us to insert icons
// We also want to be able to highlight key words using basic markdown?
// @QUESTION: At what point is it better to just use a texture for this?
// PRO: It can be anything we want for each card, it's a lot faster to draw a texture than do all of this
// PRO2: No need to save the NAME and DESCRIPTION strings as game data ... we just have them baked in texture
//      Actually we might need it anyway, for other ways of viewing the cards, like list?
// DOWNSIDE: When we want to modify a card, we have to change the data here and in the texture file ...
//      (unless we actually only have them in the texture file ...)
// DOWNSIDE2: If we go the texture route, we can not as easily animate the card text.
// My current thinking is that we stick to this in a phase where the cards may still change A LOT
// if we later decide that we for sure have not text effect and the card texts are more locked in, maybe we can
// move to textures then.
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
        DrawTextPro(
            ui_font,
            TextSubtext(text, text_pos, chara_per_line),
            VECTOR(x_pos, y_pos),
            VECTOR(0, 0),
            0.0f, font_size, 0, color);
        text_pos += chara_per_line;
        y_pos += line_height;
    }
}

void ui_command_indicator()
{
    rect_t src = {96.0f + (combat.command_points * 64.0f), 160, 64, 64};
    rect_t dest = {game_width - 160, 16, 128, 128};
    DrawTexturePro(g_ui_texture, src, dest, ZERO_VECTOR, 0.0f, WHITE);
}

void ui_discard_pile()
{
    for (u32 itr = 0; itr < g_cards_discard_num; itr++)
    {
        card_t *card = &g_card_db[g_cards_discard[itr]];
        DrawTextEx(ui_font, TextFormat("%2d: %s", itr, card->name), VECTOR(GetScreenWidth() - 256, 128 + itr * 24), 16, 0,
                   itr == g_cards_discard_num ? YELLOW : WHITE);
    }
}

void ui_command_cards()
{

    rect_t source = {0, 0, 96, 128};
    rect_t dest = {(f32)COMBAT_UI_CARD_OFFSET_X, (f32)COMBAT_UI_CARD_OFFSET_Y, COMBAT_UI_CARD_WIDTH, COMBAT_UI_CARD_HEIGHT};

    for (int itr = 0; itr < 10; itr++)
    {
        if (g_cards_hand[itr] != 0)
        {
            i32 card_id = g_cards_hand[itr];
            Color card_color = {255, 255, 255, 255};
            switch (g_card_db[card_id].unit_class)
            {
            case UNIT_CLASS_ASSEMBLER:
                card_color = DARKGREEN;
                break;
            case UNIT_CLASS_MANOWAR:
                card_color = MAROON;
                break;
            case UNIT_CLASS_SLAYER:
                card_color = ORANGE;
                break;
            case UNIT_CLASS_WIZARD:
                card_color = BLUE;
                break;
            }
            DrawTexturePro(g_ui_texture, source, dest, ZERO_VECTOR, 0.0f,
                           itr == combat.selected_card ? WHITE : itr == combat.mouse_card ? LIGHTGRAY
                                                                                          : GRAY);
            DrawTexturePro(g_ui_texture,
                           (rect_t){source.x, source.y + 128 + 32, source.width, source.height},
                           dest, ZERO_VECTOR, 0.0f,
                           itr == combat.selected_card ? card_color : itr == combat.mouse_card ? LIGHTGRAY
                                                                                               : GRAY);
            DrawTextPro(ui_font, TextFormat("[%d]%s", g_card_db[card_id].cost, g_card_db[card_id].name),
                        VECTOR(dest.x + 32, dest.y + 18), ZERO_VECTOR, 0.0f, 22, 0, card_color);
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
                DrawTexturePro(g_ui_texture, value_src, value_dest, ZERO_VECTOR, 0.0f, WHITE);
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
                    DrawTexturePro(g_ui_texture, buttons_src, buttons_dest, ZERO_VECTOR, 0.0f, WHITE);
                }
            }
        }
        else
        {
            DrawRectangleRec(dest, GRAY);
        }
        dest.x += COMBAT_UI_CARD_WIDTH;
    }
}

bool ui_pc_card(i32 id, i32 x, i32 y)
{
    player_character_t *chara = &g_player_characters[id];
    Color bg_color = {255, 255, 255, 255};
    switch (chara->unit_class)
    {
    case UNIT_CLASS_ASSEMBLER:
        bg_color = DARKGREEN;
        break;
    case UNIT_CLASS_MANOWAR:
        bg_color = MAROON;
        break;
    case UNIT_CLASS_SLAYER:
        bg_color = ORANGE;
        break;
    case UNIT_CLASS_WIZARD:
        bg_color = BLUE;
        break;
    }
    DrawTexturePro(g_ui_texture, (rect_t){320.0f, 0.0f, 128.0f, 64.0f}, (rect_t){(f32)x, (f32)y, 256.0f, 128.0f}, ZERO_VECTOR, 0.0f, LIGHTGRAY);
    DrawTextPro(ui_font, TextFormat("%s(%2d/%2d)", chara->name, chara->move_points, chara->swift), VECTOR(x + 128.0f, y + 6.0f), ZERO_VECTOR, 0.0f, 24, 0, bg_color);
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
    ui_command_indicator();
    // DrawTextEx(ui_font, TextFormat("%d", g_combat_state.command_points), VECTOR(GetScreenWidth() - 128, 16), 48, 0, WHITE);
}

void combat_update()
{

    static rect_t map_area = {COMBAT_MAP_OFFSET_X, COMBAT_MAP_OFFSET_Y,
                              COMBAT_MAP_WIDTH * COMBAT_TILE_WIDTH, COMBAT_MAP_HEIGHT * COMBAT_TILE_HEIGHT};

    combat.mouse_tile_x = (g_virtual_mouse.x - map_area.x) / COMBAT_TILE_WIDTH;
    combat.mouse_tile_y = (g_virtual_mouse.y - map_area.y) / COMBAT_TILE_HEIGHT;
    combat.mouse_tile_id = TILE_ID(combat.mouse_tile_x, combat.mouse_tile_y);

    switch (combat.state)
    {
    case COMBAT_STATE_PRE:
        break;
    case COMBAT_STATE_PLAYER_TURN_PRE:
        break;
    case COMBAT_STATE_PLAYER_MAIN:
    {
        // Is the mouse in the card area?
        if (g_virtual_mouse.y >= COMBAT_UI_CARD_OFFSET_Y)
        {
            // See which card slot we have the mouse over:
            combat.mouse_card = ((i32)g_virtual_mouse.x / COMBAT_UI_CARD_WIDTH);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // If there a card in the slot and is it not already selected?
                if (g_cards_hand[combat.mouse_card] >= 0 &&
                    combat.mouse_card != combat.selected_card)
                {
                    // Make the clicked slot our selected slot:
                    combat.selected_card = combat.mouse_card;

                    combat.button_frame = 0;
                    combat.button_timer = 0.0f;
                }
            }
        }
        else
        {
            // We are not hovering over any of the cards
            combat.mouse_card = -1;
        }

        if (combat.selected_card >= 0)
        {
            card_t *card = &g_card_db[g_cards_hand[combat.selected_card]];
            rect_t act_rec = {(f32)combat.selected_card * COMBAT_UI_CARD_WIDTH,
                              COMBAT_UI_CARD_OFFSET_Y - 64.0f,
                              64.0f,
                              64.0f};
            rect_t use_rec = act_rec;
            use_rec.x += 64;
            rect_t burn_rec = use_rec;
            burn_rec.x += 64;

            if (CheckCollisionPointRec(g_virtual_mouse, act_rec)) // mouse over "act" button
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) //"act" button pressed
                {
                    hand_discard(combat.selected_card);
                    combat.selected_card = -1;
                    combat.state = COMBAT_STATE_PLAYER_SELECTION;
                    strcpy(combat.prompt, TextFormat("Who to activate? Exhausted %s can be reactivated. \0",
                                                     unit_class_strings[card->unit_class]));
                }
            }
            if (CheckCollisionPointRec(g_virtual_mouse, use_rec)) // mouse over "use" button
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) //"use" button pressed
                {
                    if (combat.command_points >= card->cost) // if we have enough command points:
                    {
                        combat.command_points -= card->cost;                         // pay the card cost
                        combat.processing_card = g_cards_hand[combat.selected_card]; // remember the card we played
                        combat.state = COMBAT_STATE_PLAYER_PROCESS_CARD;             // and enter the card process state next frame
                        hand_discard(combat.selected_card);                          // discard selected card
                        combat.selected_card = -1;                                   // deselect selected card
                    }
                }
            }
            if (CheckCollisionPointRec(g_virtual_mouse, burn_rec)) // mouse over "burn" button
            {
                //  @TODO: Here we want to make it so that you have to hold down the mouse button for
                //  at least a full second for the burn to actually happen, giving you a chance to
                //  confirm that this is really what you want to do...
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) //"burn" button pressed
                {
                    // we can only burn cards if we have less than 12 command points:
                    if (combat.command_points < 12)
                    {
                        combat.command_points += card->value;
                        // if we had overflow, then the player doesnt get those points:
                        if (combat.command_points > 12)
                        {
                            combat.command_points = 12;
                        }
                        hand_discard(combat.selected_card); // discard selected card
                        combat.selected_card = -1;          // deselect selected card
                    }
                }
            }
        }
    }
    break;
    case COMBAT_STATE_PLAYER_PROCESS_CARD:
    {
        card_t *card = &g_card_db[combat.processing_card];
        if (IsKeyPressed(KEY_ESCAPE)) // Undo
        {
            combat.command_points += card->cost;
            undo_most_recent_discard();
            combat.state = COMBAT_STATE_PLAYER_MAIN;
        }
    }
    break;
    case COMBAT_STATE_PLAYER_SELECTION:
    {
        // Does the player wish to undo?
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) || IsKeyPressed(KEY_ESCAPE))
        {
            undo_most_recent_discard();
            combat.state = COMBAT_STATE_PLAYER_MAIN;
            strcpy(combat.prompt, "\0");
        }

        if (CheckCollisionPointRec(g_virtual_mouse, map_area))
        {
            if (combat.mouse_tile_id != -1)
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    if (g_actors[combat.mouse_tile_id].type == ACTOR_TYPE_PLAYER_UNIT)
                    {
                        combat.selected_actor_tile = combat.mouse_tile_id;
                        combat.selected_actor = g_actors[combat.selected_actor_tile].id;
                        combat.state = COMBAT_STATE_PLAYER_CONFIRM_SELECTION;
                    }
                }
            }
        }
    }
    break;
    case COMBAT_STATE_PLAYER_CONFIRM_SELECTION:
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            combat.state = COMBAT_STATE_PLAYER_SELECTION;
            combat.selected_actor = -1;
            combat.selected_actor_tile = -1;
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            // Compute BFS for selected actor:
            {
                bfs_info info = {
                    .start_x = combat.selected_actor_tile % COMBAT_MAP_WIDTH,
                    .start_y = combat.selected_actor_tile / COMBAT_MAP_WIDTH,
                    .width = COMBAT_MAP_WIDTH,
                    .height = COMBAT_MAP_HEIGHT};
                for (u32 y = 0; y < COMBAT_MAP_HEIGHT; y++)
                {
                    for (u32 x = 0; x < COMBAT_MAP_WIDTH; x++)
                    {
                        if (g_actors[y * COMBAT_MAP_WIDTH + x].type > 0)
                        {
                            info.terrain[y][x] = TERRAIN_OCCUPIED;
                        }
                    }
                }
                combat.movement_overlay = bfs_compute(info);
            }

            g_player_characters[combat.selected_actor].move_points =
                g_player_characters[combat.selected_actor].swift;

            combat.state = COMBAT_STATE_PLAYER_ACTOR_ACTIVE;
        }
    }
    break;
    case COMBAT_STATE_PLAYER_ACTOR_ACTIVE:
    {
        if (CheckCollisionPointRec(g_virtual_mouse, map_area))
        {
            if (combat.mouse_tile_id != -1)
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    if (combat.movement_overlay.costs[combat.mouse_tile_y][combat.mouse_tile_x] >= 0 &&
                        combat.movement_overlay.costs[combat.mouse_tile_y][combat.mouse_tile_x] <= g_player_characters[combat.selected_actor].move_points)
                    {
                        g_player_characters[combat.selected_actor].move_points -= combat.movement_overlay.costs[combat.mouse_tile_y][combat.mouse_tile_x];
                        combat.selected_actor_tile = actor_move(combat.selected_actor_tile, combat.mouse_tile_id);

                        // Update BFS:
                        {
                            bfs_info info = {
                                .start_x = combat.selected_actor_tile % COMBAT_MAP_WIDTH,
                                .start_y = combat.selected_actor_tile / COMBAT_MAP_WIDTH,
                                .width = COMBAT_MAP_WIDTH,
                                .height = COMBAT_MAP_HEIGHT};
                            for (u32 y = 0; y < COMBAT_MAP_HEIGHT; y++)
                            {
                                for (u32 x = 0; x < COMBAT_MAP_WIDTH; x++)
                                {
                                    if (g_actors[y * COMBAT_MAP_WIDTH + x].type > 0)
                                    {
                                        info.terrain[y][x] = TERRAIN_OCCUPIED;
                                    }
                                }
                            }
                            combat.movement_overlay = bfs_compute(info);
                        }

                        if (g_player_characters[combat.selected_actor].move_points <= 0)
                        {
                            combat.selected_actor = -1;
                            combat.selected_actor_tile = -1;
                            combat.state = COMBAT_STATE_PLAYER_MAIN;
                            strcpy(combat.prompt, "\0");
                        }
                    }
                }
            }
        }
    }
    break;
    case COMBAT_STATE_ENEMY_TURN_PRE:
        break;
    case COMBAT_STATE_ENEMY_TURN:
        break;
    case COMBAT_STATE_POST:
        break;
    }
}

void combat_render()
{
    switch (combat.state)
    {
    case COMBAT_STATE_PRE:
        break;
    case COMBAT_STATE_PLAYER_TURN_PRE:
        break;
    case COMBAT_STATE_PLAYER_SELECTION:
        // Fall through:
    case COMBAT_STATE_PLAYER_MAIN:
    {
        ClearBackground(BLACK);
        actors_render(COMBAT_MAP_OFFSET_X, COMBAT_MAP_OFFSET_Y);
        combat_ui_render();
        if (combat.prompt[0] != '\0')
        {
            vector_t text_length = MeasureTextEx(ui_font, combat.prompt, 32, 0);
            DrawTextEx(ui_font, combat.prompt, VECTOR(game_width / 2 - (text_length.x / 2), 16), 32, 0, RAYWHITE);
            text_length = MeasureTextEx(ui_font, "Press [ESC] or right mouse button to cancel.", 32, 0);
            DrawTextEx(ui_font, "Press [ESC] or right mouse button to cancel.", VECTOR(game_width / 2 - (text_length.x / 2), 48), 32, 0, LIGHTGRAY);
        }
    }
    break;
    case COMBAT_STATE_PLAYER_CONFIRM_SELECTION:
    {
        ClearBackground(BLACK);
        actors_render(COMBAT_MAP_OFFSET_X, COMBAT_MAP_OFFSET_Y);
        combat_ui_render();
        DrawRectangle(0, 0, game_width, game_height, (Color){0, 0, 0, 128});
        const char *prompt = TextFormat("Discard %s to activate %s ?", g_card_db[g_cards_discard[g_cards_discard_num - 1]].name,
                                        g_player_characters[combat.selected_actor].name);
        vector_t text_length = MeasureTextEx(ui_font, prompt, 32, 0);
        DrawTextEx(ui_font, prompt, VECTOR(game_width / 2 - (text_length.x / 2), (game_height / 4)), 32, 0, RAYWHITE);
        text_length = MeasureTextEx(ui_font, "This action can not be undone.", 32, 0);
        DrawTextEx(ui_font, "This action can not be undone.", VECTOR(game_width / 2 - (text_length.x / 2), (game_height / 4 + 48)), 32, 0, LIGHTGRAY);
    }
    break;
    case COMBAT_STATE_PLAYER_PROCESS_CARD:
    {
        ClearBackground(BLACK);
        actors_render(COMBAT_MAP_OFFSET_X, COMBAT_MAP_OFFSET_Y);
        combat_ui_render();
        DrawRectangleLines(0, 0, GetScreenWidth(), GetScreenHeight(), YELLOW);
        const char *text = TextFormat("Playing %s -- Please pick your target or press [ESC] to cancel.", g_card_db[combat.processing_card].name);
        vector_t text_length = MeasureTextEx(ui_font, text, 32, 0);
        DrawTextEx(ui_font, text, VECTOR((GetScreenWidth() / 2) - (text_length.x / 2), 16), 32, 0, YELLOW);
    }
    break;
    case COMBAT_STATE_PLAYER_ACTOR_ACTIVE:
    {
        ClearBackground(BLACK);
        actors_render(COMBAT_MAP_OFFSET_X, COMBAT_MAP_OFFSET_Y);
        combat_ui_render();
    }
    break;
    case COMBAT_STATE_ENEMY_TURN_PRE:
        break;
    case COMBAT_STATE_ENEMY_TURN:
        break;
    case COMBAT_STATE_POST:
        break;
    }
}

// NOTE (Sebbe):
// Code has been commented out of main which would allow the game
// to work in fullscreen as well as windowed mode, regardless of resolution.
// The code works fine, but using it requires that hitboxes for mouse events
// also scale accordingly, and fiddling with that now is the wrong focus for
// the prototype to move forward. The code will be brought back in when we are
// confident that we know exactly where hitboxes are going to be located.
int main()
{
    InitWindow(game_width, game_height, "Combat Prototype");
    ToggleFullscreen();
    // SetWindowMinSize(game_width / 4, game_height / 4);
    SetTargetFPS(60);
    SetExitKey(KEY_X);

    // f32 game_scale = MIN((float)GetRenderWidth() / game_width, (float)GetRenderHeight() / game_height);
    // g_virtual_mouse = Vector2Clamp(g_virtual_mouse, VECTOR(0, 0), VECTOR(game_width, game_height));

    // RenderTexture2D canvas = LoadRenderTexture(game_width, game_height);
    // SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

    g_actors[TILE_ID(0, 2)].type = ACTOR_TYPE_PLAYER_UNIT;
    g_actors[TILE_ID(0, 2)].id = 0;
    g_actors[TILE_ID(0, 3)].type = ACTOR_TYPE_PLAYER_UNIT;
    g_actors[TILE_ID(0, 3)].id = 1;
    g_actors[TILE_ID(0, 4)].type = ACTOR_TYPE_PLAYER_UNIT;
    g_actors[TILE_ID(0, 4)].id = 2;
    g_actors[TILE_ID(0, 5)].type = ACTOR_TYPE_PLAYER_UNIT;
    g_actors[TILE_ID(0, 5)].id = 3;
    g_actors[TILE_ID(0, 6)].type = ACTOR_TYPE_PLAYER_UNIT;
    g_actors[TILE_ID(0, 6)].id = 4;

    g_actors[TILE_ID(8, 1)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(7, 2)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(8, 3)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(7, 4)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(8, 5)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(7, 6)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(8, 7)].type = ACTOR_TYPE_ENEMY_UNIT;

    g_cards_hand[0] = 1;
    g_cards_hand[1] = 2;
    g_cards_hand[2] = 3;
    g_cards_hand[3] = 4;
    g_cards_hand[4] = 1;
    g_cards_hand[5] = 2;
    g_cards_hand[6] = 3;
    g_cards_hand[7] = 4;
    g_cards_hand[8] = 4;
    g_cards_hand[9] = 4;

    g_ui_texture = LoadTexture("./assets/ui.png");
    SetTextureFilter(g_ui_texture, TEXTURE_FILTER_POINT);

    ui_font = LoadFont("./assets/FiraCode-Regular.ttf");

    while (!WindowShouldClose())
    {
        /*
        if (IsKeyPressed(KEY_F))
        {
            SetWindowSize(game_width, game_height);
            ToggleFullscreen();
        }
        */

        // game_scale = MIN((float)GetRenderWidth() / game_width, (float)GetRenderHeight() / game_height);
        vector_t screen_mouse = GetMousePosition();
        g_virtual_mouse = screen_mouse;
        // g_virtual_mouse.x = screen_mouse.x / game_scale;
        // g_virtual_mouse.y = screen_mouse.y / game_scale;
        // g_virtual_mouse = Vector2Clamp(g_virtual_mouse, VECTOR(0, 0), VECTOR(game_width, game_height));

        switch (g_game_state)
        {
        case GAME_STATE_COMBAT:
        {
            combat_update();
        }
        break;
        }

        // BeginTextureMode(canvas);
        BeginDrawing();
        switch (g_game_state)
        {
        case GAME_STATE_COMBAT:
        {
            combat_render();
        }
        break;
        }
        EndDrawing();
        // EndTextureMode();

        /*
        BeginDrawing();
        {
            rect_t canvas_src = {
                .x = 0,
                .y = 0,
                .width = (f32)game_width,
                .height = -(f32)game_height};
            rect_t canvas_dest = {
                .x = 0,
                .y = 0,
                .width = (f32)GetRenderWidth(),
                .height = (f32)GetRenderHeight()};
            DrawTexturePro(canvas.texture, canvas_src, canvas_dest, VECTOR(0, 0), 0.0f, WHITE);
        }
        EndDrawing();
        */
    }
    UnloadTexture(g_ui_texture);
    UnloadFont(ui_font);
    // UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}
