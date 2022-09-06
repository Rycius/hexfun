#include "map.h"

game_map *map;

map_tile *GetMapTile(offset_coord offset)
{
    map_tile *result = 0;

    if(offset.row < 0 || offset.row > map->height - 1) return 0;

    if(!map->wrap)
    {
        if(offset.col < 0 || offset.col > map->width - 1) return 0;
    }


    uint32 address = offset.row * map->width;
    if(offset.col < 0)
    {
        int32 mod = offset.col % map->width;
        if(mod < 0)
            address += map->width + offset.col % map->width;
        else
            address += offset.col % map->width;   
    } 
    else
        address += offset.col % map->width;

    result = &map->tiles[address];

    return result;
}


offset_coord OffsetNeighbor(offset_coord offset, direction dir)
{
    offset_coord result = {0};

    offset_coord directionDifferences[2][6] = {
        // even rows 
        {Offset(+1,  0), Offset(0, -1), Offset(-1, -1), 
         Offset(-1,  0), Offset(-1, +1), Offset(0, +1)},
        // odd rows 
        {Offset(+1,  0), Offset(+1, -1), Offset(0, -1), 
         Offset(-1,  0), Offset(0, +1), Offset(+1, +1)},
    };
    
    int32 parity = offset.row & 1;
    offset_coord diff = directionDifferences[parity][dir];

    result.col = offset.col + diff.col;
    result.row = offset.row + diff.row;

    return result;
}


offset_coord *OffsetNeighbors(offset_coord offset)
{
    offset_coord *result = 0;
    arrsetcap(result, 6);

    for(int32 i = 0; i < DIR_COUNT; i++)
    {
        offset_coord neighbor = OffsetNeighbor(offset, (direction)i);
        if(neighbor.row < 0 || neighbor.row >= map->height) continue;
        
        arrput(result, neighbor);
    }

    return result;
}

// inline float CubeDistance (cube_coord a, cube_coord b)
// {
//     float result = 0;

//     result = (abs(a.q - b.q) + abs(a.r - b.r) + abs(a.s - b.s)) / 2;

//     return result;
    
// }

inline float AxialDistance(axial_coord a, axial_coord b)
{
    float result = 0;

    result = (abs(a.q - b.q) + abs(a.q + a.r - b.q - b.r) + abs(a.r - b.r)) / 2;

    return result;
}
    
inline float OffsetDistance(offset_coord a, offset_coord b)
{
    float result = 0;    

    axial_coord ax = OffsetToAxial(a);
    axial_coord bx = OffsetToAxial(b);
    result = AxialDistance(ax, bx);

    return result;
}


inline offset_coord OffsetReal(offset_coord coord)
{
    offset_coord result = {0};

    result.col = coord.col < 0 ? (map->width + (coord.col % map->width)) : (coord.col % map->width);
    result.row = coord.row;

    return result;
}

offset_coord OffsetDirDistance(offset_coord from, direction dir, int32 distance)
{
    offset_coord result = from;

    offset_coord directionDifferences[2][6] = {
        // even rows 
        {Offset(+1,  0), Offset(0, -1), Offset(-1, -1), 
         Offset(-1,  0), Offset(-1, +1), Offset(0, +1)},
        // odd rows 
        {Offset(+1,  0), Offset(+1, -1), Offset(0, -1), 
         Offset(-1,  0), Offset(0, +1), Offset(+1, +1)},
    };
    

    
    for(int32 i = 0; i < distance; i++)
    {
        int32 parity = result.row & 1;
        offset_coord diff = directionDifferences[parity][dir];

        result.col += diff.col;
        result.row += diff.row;;
    }
    

    

    return result;
}

offset_coord OffsetScale(offset_coord coord, float factor)
{
    offset_coord result = {0};

    cube_coord cube = OffsetToCube(coord);

    result = CubeToOffset(cube.q * factor, cube.r * factor, cube.s * factor);

    return result;
}

offset_coord *GetRing(offset_coord center, int32 radius)
{
    offset_coord *result = 0;

    offset_coord coord = OffsetDirDistance(center, (direction)4, radius);

    for(int32 i = 0; i < DIR_COUNT; i++)
    {
        for(int32 k = 0; k < radius; k++)
        {
            arrpush(result, coord);
            coord = OffsetNeighbor(coord, (direction)i);
        }
    }

    return result;
}

//////////////////// PATH FINDING ////////////////////////

float Heuristic(offset_coord a, offset_coord b)
{
    float result = 0.0f;

    if(map->wrap)
    {
        offset_coord aj = OffsetReal(a);
        offset_coord bj = OffsetReal(b);

        if(abs(aj.col - bj.col) > map->width / 2)
        {
            if(bj.col > aj.col) bj.col -= map->width;
            else aj.col -= map->width;
        }
        result = OffsetDistance(aj, bj);
    }
    else
    {
        result = OffsetDistance(a, b);
    }

    return result;
}

float MoveCost(map_tile *a, map_tile *b)
{
    float result = 0.0f;

    result = b->moveCost;

    return result;
}


offset_coord *FindPath(offset_coord start, offset_coord end)
{
    offset_coord *result = 0;

    path_node *pnList = 0;
    arrsetcap(pnList, map->width*map->height);

    path_node **openlist = 0;
    arrsetcap(openlist, map->width*map->height);

    path_node **closedList = 0;
    arrsetcap(closedList, map->width*map->height);

    map_tile *startTile = GetMapTile(start);
    map_tile *endTile = GetMapTile(end);

    if(!startTile || !endTile) return 0;

    path_node node = {0};
    node.coord = start;
    node.tile = startTile;
    
    arrpush(pnList, node);
    arrpush(openlist, &pnList[arrlen(pnList)-1]);
    
    path_node *currNode = 0;

    while(arrlen(openlist) > 0)
    {
        // for(int32 i = 0; i < arrlen(openlist); i++)
        // {
        //     path_node *n = openlist[i];
        //     TraceLog(LOG_INFO, "openlist x:%d, y:%d, g:%.2f, h:%.2f f:%.2f", n->tile->offset.col, n->tile->offset.row, n->g, n->h, n->f);
        // }

        float lowestF = __FLT_MAX__;
        int32 lowestFID = -1;
        for(int32 i = 0; i < arrlen(openlist); i++)
        {
            if(openlist[i]->f < lowestF) 
            {
                currNode = openlist[i];
                lowestF = currNode->f;
                lowestFID = i;
            }
        }

        arrdel(openlist, lowestFID);
        arrpush(closedList, currNode);

        if(currNode->tile == endTile)
        {
            offset_coord *path = 0;
            while(currNode->tile != startTile)
            {
                arrpush(path, currNode->coord);
                currNode = currNode->cameFrom;
            }

            for(int32 i = arrlen(path) - 1; i > -1 ; i--)
            {
                offset_coord real = OffsetReal(path[i]);
                arrpush(result, real);
            }

            arrfree(path);

            break;
        }

        offset_coord *neighborsOffsets = OffsetNeighbors(currNode->coord);

        for(int32 i = 0; i < arrlen(neighborsOffsets); i++)
        {
            //TraceLog(LOG_INFO, "Neighbor x:%d, y:%d", neighborsOffsets[i].col, neighborsOffsets[i].row);
            map_tile *neighborTile = GetMapTile(neighborsOffsets[i]);
             
            if(neighborTile == 0) continue;
            if(neighborTile->passable == false) continue;
            
            bool inClosedList = false;
            for(int32 i = 0; i < arrlen(closedList); i++)
            {
                if(closedList[i]->tile == neighborTile)
                {
                    inClosedList = true;
                    break;
                }
            }
            if(inClosedList) continue;

            bool inOpenList = false;
            int32 openIndex = -1;

            for(int32 i = 0; i < arrlen(openlist); i++)
            {
                if(openlist[i]->tile == neighborTile)
                {
                    inOpenList = true;
                    openIndex = i;
                    break;
                }
            }

            if(inOpenList == false)
            {
                path_node neighborNode = {0};
                neighborNode.coord = neighborsOffsets[i];
                neighborNode.cameFrom = currNode;
                neighborNode.g = currNode->g + MoveCost(currNode->tile, neighborTile); // +cost to move from tile to other tile
                neighborNode.h = Heuristic(neighborNode.coord, end);
                neighborNode.f = neighborNode.g + neighborNode.h;
                neighborNode.tile = neighborTile;

                arrput(pnList, neighborNode);
                arrput(openlist, &pnList[arrlen(pnList)-1]);
            }
            else
            {
                if((currNode->g + MoveCost(currNode->tile, neighborTile)) < openlist[openIndex]->g) // +cost to move from tile to other tile
                {
                    openlist[openIndex]->cameFrom = currNode;
                    openlist[openIndex]->g = currNode->g + MoveCost(currNode->tile, neighborTile); // // cost to move from tile to other tile
                    openlist[openIndex]->f = openlist[openIndex]->g + openlist[openIndex]->h;
                }
            }
        }
        arrfree(neighborsOffsets);
    }

    arrfree(pnList);
    arrfree(openlist);
    arrfree(closedList);

    return result;
}


void InitMap(int32 width, int32 height, bool wrap)
{
    if(map) free(map);
    
    map = (game_map *)MemAlloc(sizeof(game_map));

    arrsetcap(map->tiles, map->width * map->height);
    
    map->width = width;
    map->height = height;
    map->wrap = wrap;


    for(int32 y = 0; y < map->height; y++)
    {
        for(int32 x = 0; x < map->width; x++)
        {
            map_tile tile = {0};

            tile.coord.col = x;
            tile.coord.row = y;

            arrput(map->tiles, tile);
        }
    }
}


void GenerateTerrain()
{
    
    SetRandomSeed(GetTime());

    for(int32 y = 0; y < map->height; y++)
    {
        for(int32 x = 0; x < map->width; x++)
        {
            map_tile *tile = GetMapTile(Offset(x, y));

            tile->type = MAP_TILE_GRASSLAND;

            int32 passRand = GetRandomValue(0, 10);
            tile->passable = passRand > 2 ? true : false;
            if(tile->passable)
            {
                int32 hillRand = GetRandomValue(0, 10);
                tile->moveCost = hillRand > 3 ? 1.0f : 2.0f;
                if(tile->moveCost > 1.0f)
                    tile->type = MAP_TYLE_GRASSLAND_HILL;
            }

            if(y > 5 && y < 10 && x > 3 && x < 10)
            {
                tile->type = MAP_TILE_DESERT;
            }
        }
    }
}