#include <cstdint>

#include "raylib.h"
#include "raymath.h"

#include "stb_ds.h"



#define int8   int8_t
#define uint8  uint8_t
#define int16  int16_t
#define uint16 uint16_t
#define int32  int32_t
#define uint32 uint32_t
#define int64  int64_t 
#define uint64 uint64_t
#define ptr    size_t

#define v2 Vector2
#define v3 Vector3
#define v4 Vector4


#if MY_DEBUG
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))




#define SQRT3 1.73205080757f
#define HEX_SIZE 40.0f
#define HEX_HEIGHT (HEX_SIZE * 2.0f)
#define HEX_WIDTH (SQRT3 * HEX_SIZE)
#define HEX_CENTRE_DIST_VERT (0.75f * HEX_HEIGHT)
#define HEX_CENTRE_DIST_HOR HEX_WIDTH

#define HEX_TEXTURE_SIZE 512
#define HEX_TEXTURE_HEX_SIZE (HEX_TEXTURE_SIZE / 2)

#define UNITS_PER_PLAYER_CAP 100
#define CITIES_PER_PLAYER_CAP 100

inline v2 Vec2()
{
    v2 result = (v2){0.0f, 0.0f};

    return result;
}

inline v2 Vec2(float x, float y)
{
    v2 result = (v2){x, y};

    return result;
}

inline v3 Vec3(float x, float y, float z)
{
    v3 result = (v3){x, y, z};

    return result;
}

inline v4 Vec4(float x, float y, float z, float w)
{
    v4 result = (v4){x, y, z, w};

    return result;
}

inline Rectangle Rec(float x, float y, float width, float height)
{
    Rectangle result = (Rectangle){x, y, width, height};

    return result;
}

inline Rectangle Rec(v2 pos, v2 size)
{
    Rectangle result = {pos.x, pos.y, size.x, size.y};

    return result;
}

inline Rectangle Rec(v2 pos, float width, float height)
{
    Rectangle result = {pos.x, pos.y, width, height};

    return result;
}

inline Rectangle Rec(float x, float y, v2 dim)
{
    Rectangle result = (Rectangle){x, y, dim.x, dim.y};

    return result;
}

inline v2 RecPos(Rectangle rec)
{
    v2 result = {rec.x, rec.y};

    return result;
}

inline v2 RecDim(Rectangle rec)
{
    v2 result = {rec.width, rec.height};

    return result;
}

inline int Clamp(int value, int min, int max)
{
    if(value < min) return min;
    if(value >= max) return max - 1;
    return value;
}



enum direction {DIR_EAST, DIR_NORTH_EAST, DIR_NORTH_WEST, DIR_WEST, DIR_SOUTH_WEST, DIR_SOUTH_EAST, DIR_COUNT};


struct cube_coord
{
    int32 q;
    int32 r;
    int32 s;
};

struct axial_coord
{
    int32 q;
    int32 r;
};

struct offset_coord
{
    int32 col;
    int32 row;
};

inline offset_coord Offset(int32 c, int32 r)
{
    offset_coord result = {0};

    result.col = c;
    result.row = r;

    return result;
}

inline cube_coord Cube(int32 q, int32 r, int32 s)
{
    cube_coord result = {0};

    result.q = q;
    result.r = r;
    result.s = s;

    return result;
}

enum game_unit_type { UNIT_TYPE_WARRIOR, UNIT_TYPE_COUNT };

struct game_unit
{
    offset_coord coord;
    game_unit_type type;
    Rectangle rec;
    float transition;
    bool moving;
    offset_coord *path;
    float movement;
    float movementLeft;
};

enum map_terrain_type
{
    TERRAIN_TYPE_OCEAN,
    TERRAIN_TYPE_LAKE,
    TERRAIN_TYPE_COAST,
    TERRAIN_TYPE_GRASSLAND,
    TERRAIN_TYPE_PLAIN,
    TERRAIN_TYPE_DESERT,
    TERRAIN_TYPE_TUNDRA,
    TERRAIN_TYPE_SNOW,

    TERRAIN_TYPE_COUNT
};

enum map_terrain_modifier
{
    TERRAIN_MODIFIER_NONE,
    TERRAIN_MODIFIER_HILL,
    TERRAIN_MODIFIER_MOUNTAIN,

    TERRAIN_MODIFIER_COUNT
};

enum map_terrain_feature
{
    TERRAIN_FEATURE_FOREST,

    TERRAIN_FEATURE_COUNT
};

struct map_terrain
{
    map_terrain_type type;
    map_terrain_modifier modifier;
};

struct game_player;

struct game_city
{
    game_player *owner;
    offset_coord coord;
    offset_coord *area;
    v2 *areaLine;
};

struct map_tile
{
    offset_coord coord;
    map_terrain terrain;
    game_unit *unit;
    game_city *city;
    bool showPath;
    bool passable;
    float moveCost;
};

struct game_map
{
    int32 width;
    int32 height;
    map_tile *tiles;
    bool wrap;
};

struct game_player
{
    const char *name;
    Color color;
    game_unit *units;
    game_city *cities;
};

struct game_data
{
    game_map *map;
    game_player *players;
};
