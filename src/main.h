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

inline Rectangle Rec(int x, int y, int width, int height)
{
    Rectangle result = (Rectangle){x, y, width, height};

    return result;
}

inline int Clamp(int value, int min, int max)
{
    if(value < min) return min;
    if(value > max) return max;
    return value;
}


enum direction {DIR_EAST, DIR_NORTH_EAST, DIR_NORTH_WEST, DIR_WEST, DIR_SOUTH_WEST, DIR_SOUTH_EAST, DIR_COUNT};



struct offset_coord
{
    int32 col;
    int32 row;
};


struct game_unit
{
    offset_coord coord;
    offset_coord *path;
};

struct map_tile
{
    offset_coord offset;
    Texture texture;
    Color overlayColor;
    game_unit *unit;
    bool showPath;
};

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

struct path_node
{
    offset_coord offset;
    float f;
    float g;
    float h;
    path_node *cameFrom;
    map_tile *tile;
};