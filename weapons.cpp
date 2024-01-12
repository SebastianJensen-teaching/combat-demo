#include "common.h"

enum weapon_index_e : u8
{
    WEAPON_KNIFE,
    WEAPON_MACHETE,
    WEAPON_MONOFI_BLADE,
    WEAPON_SHOCK_BATON,
    WEAPON_CLUB,
    WEAPON_ELECTRO_FLAIL,
    WEAPON_REVOLVER,
    WEAPON_AUTOPISTOL,
    WEAPON_HANDCANNON,
    WEAPON_NUM_WEAPONS
};

enum weapon_type_e : u8
{
    WEAPON_TYPE_MELEE,
    WEAPON_TYPE_REVOLVER,
    WEAPON_TYPE_PISTOL,
    WEAPON_TYPE_AUTOPISTOL,
    WEAPON_TYPE_SMG,
    WEAPON_TYPE_ASSAULT,
    WEAPON_TYPE_RIFLE,
    WEAPON_TYPE_SNIPER_RIFLE,
    WEAPON_TYPE_SHOTGUN,
    WEAPON_TYPE_SPRAYER,
    WEAPON_TYPE_HOMING,
    WEAPON_TYPE_BEAM
};

enum ammo_type_e : u8
{
    AMMO_38CAL,
    AMMO_357CAL,
    AMMO_45CAL,
    AMMO_9MM,
    AMMO_762MM
};

struct weapon_data_t
{
    weapon_type_e type;
    u8 acc_pen_moving;
    u8 acc_pen_melee;
    u8 acc_pen_far;
    u8 range;
    u8 damage[3];
    ammo_type_e ammo;
    u8 mag_size;
    u8 burst_count;
    u8 armor_pen;
    u8 snap_shot_cost;
    u8 aim_shot_cost;
    u8 burst_cost;
    u8 quality;
    u16 weight;
    u16 cost;
};

struct weapon_ui_t
{
    const char *text;
    rect_t source;
    vector_t offset;
};

weapon_data_t weapon_db[WEAPON_NUM_WEAPONS] = {};
weapon_ui_t weapon_ui_info[WEAPON_NUM_WEAPONS] = {};
