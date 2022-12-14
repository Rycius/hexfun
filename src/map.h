

inline axial_coord CubeToAxial(cube_coord cube)
{
    axial_coord result = {0};

    result.q = cube.q;
    result.r = cube.r;

    return result;
}

inline cube_coord AxialToCube(axial_coord axial)
{
    cube_coord result = {0};

    result.q = axial.q;
    result.r = axial.r;
    result.s = -axial.q - axial.r;

    return result;
}

inline offset_coord AxialToOffset(axial_coord axial)
{
    offset_coord result = {0};

    result.col = axial.q + (axial.r - (axial.r & 1)) / 2;
    result.row = axial.r;

    return result;
}

inline axial_coord OffsetToAxial(offset_coord coord)
{
    axial_coord result = {0};

    result.q = coord.col - (coord.row - (coord.row & 1)) / 2;
    result.r = coord.row;

    return result;
}

inline offset_coord CubeToOffset(cube_coord cube)
{
    offset_coord result = {0};

    result.col = cube.q + (cube.r - (cube.r & 1)) / 2;
    result.row = cube.r;

    return result;
}

inline cube_coord OffsetToCube(offset_coord coord)
{
    cube_coord result = {0};

    result.q = coord.col - (coord.row - (coord.row & 1)) / 2;
    result.r = coord.row;
    result.s = -result.q - result.r;

    return result;
}

inline offset_coord CubeToOffset(int32 q, int32 r, int32 s)
{
    offset_coord result = CubeToOffset(Cube(q, r, s));

    return result;
}

inline cube_coord OffsetToCube(int32 q, int32 r)
{
    cube_coord result = OffsetToCube(Offset(q, r));

    return result;
}

inline cube_coord CubeRound(float q, float r, float s)
{
    cube_coord result = {0};

    result.q = round(q);
    result.r = round(r);
    result.s = round(s);

    float qDiff = abs(result.q - q);
    float rDiff = abs(result.r - r);
    float sDiff = abs(result.s - s);

    if (qDiff > rDiff && qDiff > sDiff)
        result.q = -result.r - result.s;
    else if(rDiff > sDiff)
        result.r = -result.q - result.s;
    else 
        result.s = -result.q - result.r;

    return result;
}

inline axial_coord AxialRound(float q, float r)
{
    axial_coord result = {0};


    cube_coord cube = CubeRound(q, r, -q - r);

    result = CubeToAxial(cube);

    return result;
}

inline axial_coord ScreenToAxial(v2 pos)
{
    axial_coord result = {0};

    float q = (SQRT3 / 3.0f * pos.x - 1.0f / 3.0f * pos.y) / HEX_SIZE;
    float r = (2.0f / 3.0f * pos.y) / HEX_SIZE;

    result = AxialRound(q, r);

    return result;
}

inline v2 AxialToScreen(axial_coord axial)
{
    v2 result = {0};

    result.x = HEX_SIZE * (SQRT3 * axial.q + SQRT3 / 2.0f * axial.r);
    result.y = HEX_SIZE * (3.0f / 2.0f * axial.r);

    return result;
}

inline offset_coord ScreenToOffset(v2 pos)
{
    offset_coord result = {0};

    result = AxialToOffset(ScreenToAxial(pos));

    return result;
}

inline v2 OffsetToScreen(offset_coord coord)
{
    v2 result = {0};

    result.x = HEX_SIZE * SQRT3 * (coord.col + 0.5f * (coord.row&1));
    result.y = HEX_SIZE * 3.0f / 2.0f * coord.row;

    return result;
}

inline v2 RealOffsetToScreen(offset_coord coord, Camera2D camera, int32 mapWidth)
{
    v2 result = {0};

    v2 leftTopPixel = Vector2Subtract(camera.target, camera.offset);
    int32 midCol = ScreenToOffset(Vec2(leftTopPixel.x + GetScreenWidth() / 2, leftTopPixel.y)).col;
    int32 page = round((float)midCol / (float)mapWidth);

    coord.col += page * mapWidth;
    result = OffsetToScreen(coord);

    return result;
}

inline offset_coord OffsetWrapAdjusted(offset_coord coord, Camera2D camera)
{
    offset_coord result = coord;

    v2 topLeftPixel = Vector2Subtract(camera.target, camera.offset);
    v2 screen = OffsetToScreen(coord);
    v2 screenAdjusted = Vector2Add(screen, topLeftPixel);
    result = ScreenToOffset(screenAdjusted);
    result.row = coord.row;

    return result;
}

struct path_node
{
    offset_coord coord;
    float f;
    float g;
    float h;
    path_node *cameFrom;
    map_tile *tile;
};