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

struct game_unit
{
    offset_coord coord;
    float transition;
    bool moving;
    offset_coord *path;
    float movement;
    float movementLeft;
};

enum map_tile_type
{
    MAP_TILE_GRASSLAND,
    MAP_TYLE_GRASSLAND_HILL,
    MAP_TILE_DESERT,
    MAP_TILE_MOUNTAIN,

    MAP_TILE_COUNT
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
    map_tile_type type;
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

struct game_state
{
    game_map *map;
    game_player *players;
};
