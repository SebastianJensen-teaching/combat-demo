#include "common.h"
#include "pathfinding.h"

#define COMBAT_MAP_WIDTH 9
#define COMBAT_MAP_HEIGHT 9
#define COMBAT_MAP_OFFSET_X 512
#define COMBAT_MAP_OFFSET_Y 128

#define COMBAT_TILES_NUM (COMBAT_MAP_WIDTH * COMBAT_MAP_HEIGHT)
#define COMBAT_TILE_WIDTH 64
#define COMBAT_TILE_HEIGHT 64

#define COMBAT_DECK_SIZE 60
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
    i32 current_turn;
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
    .current_turn = 1,
    .state = COMBAT_STATE_PLAYER_TURN_PRE,
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
    UNIT_CLASS_WIZARD,
    UNIT_CLASS_COMMANDER
};

Color class_colors[] = {
    (Color){2, 245, 148, 255},  // Assembler
    (Color){250, 53, 15, 255},  // Slayer
    (Color){245, 125, 2, 255},  // Manowar
    (Color){0, 185, 245, 255},  // Wizard
    (Color){132, 2, 245, 255}}; // Commander

const char *class_strings[] = {
    "ASSEMBLER",
    "SLAYER",
    "MANOWAR",
    "WIZARD",
    "COMMANDER"};

enum player_character_state : u8
{
    PC_STATE_EMPTY,
    PC_STATE_DEAD,
    PC_STATE_READY,
    PC_STATE_ACTIVE,
    PC_STATE_EXHAUSTED,
    PC_STATE_REACTIVATED
};

struct player_character_t
{
    char name[24];
    u8 state;
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
     .state = PC_STATE_READY,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Gentleman\0",
     .unit_class = UNIT_CLASS_WIZARD,
     .state = PC_STATE_READY,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Mercenary\0",
     .unit_class = UNIT_CLASS_MANOWAR,
     .state = PC_STATE_READY,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Navvie\0",
     .unit_class = UNIT_CLASS_SLAYER,
     .state = PC_STATE_READY,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
    {.name = "Preacher\0",
     .unit_class = UNIT_CLASS_COMMANDER,
     .state = PC_STATE_READY,
     .brains = 4,
     .brawn = 4,
     .swift = 4,
     .guts = 4,
     .hit_points = 16},
};

// ACTORS:
#include "actor.cpp"

#include "combat-cards.cpp"

void turn_end()
{
    for (int itr = 0; itr < COMBAT_HAND_SIZE; itr++)
    {
        if (g_cards_hand[itr])
        {
            combat.command_points += g_card_db[g_cards_hand[itr]].value;
            card_discard(itr);
        }
    }
    if (combat.command_points > 12)
        combat.command_points = 12;
    combat.state = COMBAT_STATE_ENEMY_TURN;
}

#include "combat-ui.cpp"

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
    {
        // Refresh all player characters and draw cards:
        i32 cards_drawn = 0;
        for (int itr = 0;
             itr < sizeof(g_player_characters) / sizeof(player_character_t);
             itr++)
        {
            if (g_player_characters[itr].state > PC_STATE_DEAD)
            {
                g_player_characters[itr].state = PC_STATE_READY;
                if (!card_draw())
                {
                    // defeat();
                } // if we cannot draw a card then we have lost
                cards_drawn++;
            }
        }
        if (!cards_drawn)
        {
            // if we have no cards at this point then we have lost (everyone is dead)
            // defeat();
        }
        // @TODO: Here we should draw extra cards from player bonueses

        // Reset UI:
        {
            combat.mouse_card = -1;
            combat.selected_actor = -1;
            combat.selected_card = -1;
        }

        // Auto move to player main state:
        combat.state = COMBAT_STATE_PLAYER_MAIN;
    }
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
                    card_discard(combat.selected_card);
                    combat.selected_card = -1;
                    combat.state = COMBAT_STATE_PLAYER_SELECTION;
                    strcpy(combat.prompt, TextFormat("Who to activate? Exhausted %s can be reactivated. \0",
                                                     class_strings[card->unit_class]));
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
                        card_discard(combat.selected_card);                          // discard selected card
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
                        card_discard(combat.selected_card); // discard selected card
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
                        if (g_player_characters[combat.selected_actor].state == PC_STATE_READY)
                        {
                            combat.state = COMBAT_STATE_PLAYER_CONFIRM_SELECTION;
                        }
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

            g_player_characters[combat.selected_actor].state = PC_STATE_ACTIVE;
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
                            g_player_characters[combat.selected_actor].state = PC_STATE_EXHAUSTED;
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
        combat.current_turn++;
        combat.state = COMBAT_STATE_PLAYER_TURN_PRE;
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
            draw_text(ui_font, combat.prompt, VECTOR(game_width / 2 - (text_length.x / 2), 16), ZERO_VECTOR, 0.0f, 32, 0, RAYWHITE);
            text_length = MeasureTextEx(ui_font, "Press [ESC] or right mouse button to cancel.", 32, 0);
            draw_text(ui_font, "Press [ESC] or right mouse button to cancel.", VECTOR(game_width / 2 - (text_length.x / 2), 48), ZERO_VECTOR, 0.0f, 32, 0, LIGHTGRAY);
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
        draw_rectangle_lines(0, 0, GetScreenWidth(), GetScreenHeight(), YELLOW);
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

void combat_load_resources()
{
    g_ui_texture = LoadTexture("./assets/ui.png");
    SetTextureFilter(g_ui_texture, TEXTURE_FILTER_POINT);

    ui_font = LoadFont("./assets/FiraCode-Regular.ttf");
}

void combat_setup_battlefield()
{
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
    g_actors[TILE_ID(8, 1)].id = 0;
    g_actors[TILE_ID(7, 2)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(7, 2)].id = 1;
    g_actors[TILE_ID(8, 3)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(8, 3)].id = 2;
    g_actors[TILE_ID(7, 4)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(7, 4)].id = 3;
    g_actors[TILE_ID(8, 5)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(8, 5)].id = 4;
    g_actors[TILE_ID(7, 6)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(7, 6)].id = 5;
    g_actors[TILE_ID(8, 7)].type = ACTOR_TYPE_ENEMY_UNIT;
    g_actors[TILE_ID(8, 7)].id = 6;
}

int main()
{
    init_window(game_width, game_height);

    combat_load_resources();
    combat_setup_battlefield();

    set_random_seed((u32)time(NULL));
    fill_deck_with_random_cards();

    while (!WindowShouldClose())
    {
        vector_t screen_mouse = GetMousePosition();
        g_virtual_mouse = screen_mouse;

        switch (g_game_state)
        {
        case GAME_STATE_COMBAT:
        {
            combat_update();
        }
        break;
        }

        begin_drawing();
        switch (g_game_state)
        {
        case GAME_STATE_COMBAT:
        {
            combat_render();
        }
        break;
        }
        end_drawing();
    }
    UnloadTexture(g_ui_texture);
    UnloadFont(ui_font);
    CloseWindow();
    return 0;
}
