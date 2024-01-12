#include "common.h"



enum command_cards
{
    CARD_NO_CARD,
    CARD_SHADOW,
    CARD_HACK,
    CARD_DISARM,
    CARD_FILCH,
    CARD_CONCENTRATE,
    CARD_TELEKINESIS,
    CARD_BARRIER,
    CARD_SWEET_LEAF,
    CARD_BLOODRUSH,
    CARD_THRUST,
    CARD_DASH,
    CARD_AGGRO,
    CARD_NUM_CARDS
};

struct card_t
{
    u8 cost;
    u8 value;
    u8 unit_class;
    bool selection_sequence[TARGET_NUM_TARGETS];
    const char *name;
    const char *description;
};

card_t g_card_db[] = {
    {.name = "NO_CARD"},
    {.cost = 1,
     .name = "SHADOW",
     .value = 1,
     .unit_class = UNIT_CLASS_ASSEMBLER,
     .description = "ASSEMBLER remains hidden until next activated or discovered",
     .selection_sequence[TARGET_ASSEMBLER] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_ASSEMBLER,
     .name = "HACK",
     .description = "ASSEMBLER attempts to hack adjacent TERMINAL",
     .selection_sequence[TARGET_ASSEMBLER] = true,
     .selection_sequence[TARGET_ADJ_TERMINAL] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_ASSEMBLER,
     .name = "DISARM",
     .description = "ASSEMBLER attempts to disarm adjacent TRAP",
     .selection_sequence[TARGET_ASSEMBLER] = true,
     .selection_sequence[TARGET_ADJ_TRAP] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_ASSEMBLER,
     .name = "FILCH",
     .description = "ASSEMBLER attempts to steal item from adjacent ENEMY",
     .selection_sequence[TARGET_ASSEMBLER] = true,
     .selection_sequence[TARGET_ADJ_ENEMY] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_WIZARD,
     .name = "CONCENTRATE",
     .description = "WIZARD gains one ESP",
     .selection_sequence[TARGET_WIZARD] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_WIZARD,
     .name = "TELEKINESIS",
     .description = "WIZARD pushes adjacent target away",
     .selection_sequence[TARGET_WIZARD] = true,
     .selection_sequence[TARGET_ADJ_CHARACTER] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_WIZARD,
     .name = "BARRIER",
     .description = "WIZARD creates a BARRIER with 3xESP HP",
     .selection_sequence[TARGET_WIZARD] = true,
     .selection_sequence[TARGET_ADJ_UNOCCUPIED] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_WIZARD,
     .name = "SWEET LEAF",
     .description = "WIZARD heals self by ESP",
     .selection_sequence[TARGET_WIZARD] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_SLAYER,
     .name = "BLOODRUSH",
     .description = "SLAYER gains one adrenaline",
     .selection_sequence[TARGET_SLAYER] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_SLAYER,
     .name = "THRUST",
     .description = "SLAYER adds their adrenaline to their attack this turn",
     .selection_sequence[TARGET_SLAYER] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_SLAYER,
     .name = "DASH",
     .description = "SLAYER doubles their movement this turn",
     .selection_sequence[TARGET_SLAYER] = true},
    {.cost = 1,
     .value = 1,
     .unit_class = UNIT_CLASS_SLAYER,
     .name = "AGGRO",
     .description = "ALL SLAYERS attack one random adjacent target",
     .selection_sequence[TARGET_NO_TARGET] = true},
};

u32 g_cards_deck[COMBAT_DECK_SIZE];
i32 g_deck_top = COMBAT_DECK_SIZE;
u32 g_cards_discard[COMBAT_DECK_SIZE];
u32 g_cards_discard_num;
i32 g_recent_discard_came_from;
i32 g_cards_hand[COMBAT_HAND_SIZE];

void fill_deck_with_random_cards()
{
    for (i32 itr = 0; itr < COMBAT_DECK_SIZE; itr++)
    {
        g_cards_deck[itr] = GetRandomValue(1, CARD_NUM_CARDS - 1);
        printf("%d: %s\n", itr, g_card_db[g_cards_deck[itr]].name);
    }
}

bool card_draw()
{
    if (g_deck_top - 1 < 0)
        return false; // no cards left to draw
    for (i32 itr = 0;
         itr < (sizeof(g_cards_hand) / sizeof(i32));
         itr++)
    {
        if (!g_cards_hand[itr])
        {
            g_deck_top--;
            g_cards_hand[itr] = g_cards_deck[g_deck_top];
            g_cards_deck[g_deck_top] = CARD_NO_CARD;
            return true;
        }
    }
    return false; // no empty card slots left
}

void card_discard(i32 card)
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