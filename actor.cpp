#define ACTOR_NOT_FOUND -1

enum actor_type
{
    ACTOR_NO_ACTOR,
    ACTOR_TYPE_PLAYER_UNIT,
    ACTOR_TYPE_ENEMY_UNIT,
    ACTOR_TYPE_TRAP,
    ACTOR_TYPE_TERMINAL,
    ACTOR_TYPE_BARRIER
};

struct actor_t
{
    actor_type type;
    u32 id;
};

actor_t g_actors[COMBAT_TILES_NUM];

i32 actor_find_tile_id(u32 id)
{
    for (u32 itr; itr < COMBAT_TILES_NUM; itr++)
    {
        if (g_actors[itr].id == id)
        {
            return itr;
        }
    }
    return -1; // no actor found
}

i32 actor_move(u32 src_id, u32 dest_id)
{
#ifdef ENABLE_CHECKS
    if (src_id > COMBAT_TILES_NUM - 1 || dest_id > COMBAT_TILES_NUM - 1)
    {
        return -1;
    }
    if (g_actors[dest_id].type != ACTOR_NO_ACTOR)
    {
        return -1;
    }
#endif
    g_actors[dest_id] = g_actors[src_id];
    g_actors[src_id] = (actor_t){.type = ACTOR_NO_ACTOR, .id = 0};
    return dest_id;
}

i32 actor_move_by_actor_id(u32 actor_id, u32 tile_id)
{
    i32 actor = actor_find_tile_id(actor_id);
#ifdef ENABLE_CHECKS
    if (actor == ACTOR_NOT_FOUND)
    {
        return -1;
    }
#endif
    return actor_move(actor, tile_id);
}

void actors_render(i32 offset_x, i32 offset_y)
{
    // Draw base grid:
    for (int y = 0; y < COMBAT_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < COMBAT_MAP_WIDTH; x++)
        {
            DrawRectangle(x * COMBAT_TILE_WIDTH + offset_x, y * COMBAT_TILE_HEIGHT + offset_y,
                          COMBAT_TILE_WIDTH, COMBAT_TILE_HEIGHT, (y * COMBAT_MAP_WIDTH + x) % 2 == 0 ? GRAY : DARKGRAY);
        }
    }

    // Draw movement overlay if there is a selected actor:
    if (combat.state == COMBAT_STATE_PLAYER_ACTOR_ACTIVE)
    {
        for (int y = 0; y < COMBAT_MAP_HEIGHT; y++)
        {
            for (int x = 0; x < COMBAT_MAP_WIDTH; x++)
            {
                if (combat.movement_overlay.costs[y][x] >= 0 &&
                    combat.movement_overlay.costs[y][x] <= g_player_characters[combat.selected_actor].move_points)
                {
                    DrawRectangle(x * COMBAT_TILE_WIDTH + offset_x, y * COMBAT_TILE_HEIGHT + offset_y,
                                  COMBAT_TILE_WIDTH, COMBAT_TILE_HEIGHT, (Color){0, 0, 255, 128});
                }
            }
        }
    }

    for (int y = 0; y < COMBAT_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < COMBAT_MAP_WIDTH; x++)
        {
            actor_t *current_actor = &g_actors[y * COMBAT_MAP_WIDTH + x];
            switch (current_actor->type)
            {
            case ACTOR_NO_ACTOR:
                continue;
            case ACTOR_TYPE_PLAYER_UNIT:
            {
                player_character_t *chara = &g_player_characters[current_actor->id];

                Color pc_color = class_colors[chara->unit_class];

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
                    pc_color.r *= brightness[chara->state];
                    pc_color.g *= brightness[chara->state];
                    pc_color.b *= brightness[chara->state];
                }
                i32 center_x = (x * COMBAT_TILE_WIDTH) + (COMBAT_TILE_WIDTH / 2) + offset_x;
                i32 center_y = (y * COMBAT_TILE_HEIGHT) + (COMBAT_TILE_HEIGHT / 2) + offset_y;
                DrawCircle(center_x, center_y, COMBAT_TILE_WIDTH * 0.40f,
                           pc_color);
                DrawText(TextFormat("%02d", current_actor->id), center_x - 8, center_y - 8, 18, RAYWHITE);
            }
            break;

            case ACTOR_TYPE_ENEMY_UNIT:
            {
                DrawCircle((x * COMBAT_TILE_WIDTH) + (COMBAT_TILE_WIDTH / 2) + offset_x,
                           (y * COMBAT_TILE_HEIGHT) + (COMBAT_TILE_HEIGHT / 2) + offset_y,
                           COMBAT_TILE_WIDTH * 0.40f, RED);
            }
            break;
            }
        }
    }
}