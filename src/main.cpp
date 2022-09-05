#define MY_DEBUG 1

#define STB_DS_IMPLEMENTATION
#include "main.h"
#include "gui.cpp"
#include "map.cpp"


v2 PointyHexCorner(v2 center, float size, int32 i)
{
    v2 result = Vec2();

    float angleDeg = 60.0f * i - 30.0f;
    float angleRad = PI / 180 * angleDeg;

    result.x = center.x + size * cosf(angleRad);
    result.y = center.y + size * sinf(angleRad);

    return result;
}

bool ArrContainsOffset(offset_coord *arr, offset_coord coord)
{
    bool result = false;

    for(int32 i = 0; i < arrlen(arr); i++)
    {
        if(arr[i].col == coord.col && arr[i].row == coord.row)
        {
            result = true;
            break;
        }
    }

    return result;
}

inline v2 *HexLine(v2 center, direction dir)
{
    v2 *result = {0};

    v2 corner1 = {0};
    v2 corner2 = {0};

    switch (dir)
    {
    case DIR_EAST:
        corner1 = PointyHexCorner(center, HEX_SIZE, 0);
        corner2 = PointyHexCorner(center, HEX_SIZE, 1);
    break; 
    
    case DIR_NORTH_EAST:
        corner1 = PointyHexCorner(center, HEX_SIZE, 5);
        corner2 = PointyHexCorner(center, HEX_SIZE, 0);
    break; 
    
    case DIR_NORTH_WEST:
        corner1 = PointyHexCorner(center, HEX_SIZE, 4);
        corner2 = PointyHexCorner(center, HEX_SIZE, 5);
    break;
    
    case DIR_WEST:
        corner1 = PointyHexCorner(center, HEX_SIZE, 3);
        corner2 = PointyHexCorner(center, HEX_SIZE, 4);
    break;

    case DIR_SOUTH_WEST:
        corner1 = PointyHexCorner(center, HEX_SIZE, 2);
        corner2 = PointyHexCorner(center, HEX_SIZE, 3);
    break;
    
    case DIR_SOUTH_EAST:
        corner1 = PointyHexCorner(center, HEX_SIZE, 1);
        corner2 = PointyHexCorner(center, HEX_SIZE, 2);
    break;
    
    default:
        break;
    }
    
    arrpush(result, corner1);
    arrpush(result, corner2);

    return result;
}



v2 *CityAreaLine(game_city *city)
{
    v2 *result = 0;

    for(int32 i = 0; i < arrlen(city->area); i++)
    {
        v2 dir = Vector2Normalize(Vector2Subtract(OffsetToScreen(city->area[i]), OffsetToScreen(city->coord)));
        offset_coord *neighbors = OffsetNeighbors(city->area[i]);

        for(int32 n = 0; n < arrlen(neighbors); n++)
        {
            if(neighbors[n].col == city->coord.col && neighbors[n].row == city->coord.row) continue;
            if(ArrContainsOffset(city->area, neighbors[n])) continue;

            v2 *line = HexLine(Vector2Scale(dir, HEX_CENTRE_DIST_HOR), (direction)n);
            v2 p1 = line[0];
            v2 p2 = line[1];
            arrpush(result, p1);
            arrpush(result, p2);
            arrfree(line);
            
        }

        arrfree(neighbors);
    }

    return result;
}

bool IsOnScreen(v2 pos, v2 dim, Camera2D camera)
{
    bool result = false;

    v2 zoomDim = Vector2Scale(dim, camera.zoom);
    v2 screenPos = GetWorldToScreen2D(pos, camera);
    Rectangle screenRec = Rec(screenPos.x - zoomDim.x/2.0f, screenPos.y - zoomDim.y/2.0f, zoomDim.x, zoomDim.y);
    
    result = CheckCollisionRecs(screenRec, Rec(0.0f, 0.0f, GetScreenWidth(), GetScreenHeight()));

    return result;
}


int main() 
{

    // Image img = GenImageColor(HEX_TEXTURE_SIZE, HEX_TEXTURE_SIZE, Fade(WHITE, 0.0f));

    // v2 prevPoint = PointyHexCorner(Vec2(HEX_TEXTURE_SIZE/2, HEX_TEXTURE_SIZE/2), HEX_TEXTURE_HEX_SIZE, 0);
    // for(int32 i = 1; i < 6; i++)
    // {
    //     v2 point = PointyHexCorner(Vec2(HEX_TEXTURE_SIZE/2, HEX_TEXTURE_SIZE/2), HEX_TEXTURE_HEX_SIZE, i);
    //     ImageDrawLineV(&img, prevPoint, point, BLACK); 
    //     prevPoint = point;
    // } 
    // ImageDrawLineV(&img, prevPoint, PointyHexCorner(Vec2(HEX_TEXTURE_SIZE/2, HEX_TEXTURE_SIZE/2), HEX_TEXTURE_HEX_SIZE, 0), BLACK); 
    // ExportImage(img, "img.png");

    // Initialization
    //--------------------------------------------------------------------------------------
    const int32 screenWidth = 1600;
    const int32 screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "raylib");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second


    Camera2D camera = {0};
    camera.zoom = 1.0f;
    camera.offset.x = 0.0f; 
    camera.offset.y = 0.0f; 

    float cameraSpeed = 10.0f;
    float cameraZoomSpeed = 1.0f;
    float cameraZoomTarget = 1.0f;

    game_map *map = (game_map *)MemAlloc(sizeof(game_map));
    InitMap(map, 44, 26, true);
    GenerateTerrain(map);


    map_tile *unitTile = GetMapTile(map, Offset(5, 5));    
    unitTile->unit = (game_unit *)MemAlloc(sizeof(game_unit));
    unitTile->unit->coord = unitTile->coord;
    

    int32 mapWidthInPixels = HEX_CENTRE_DIST_HOR * map->width + HEX_WIDTH / 2.0f;
    int32 mapHeightInPixels = HEX_CENTRE_DIST_VERT * map->height + HEX_HEIGHT * 0.25f;


    v2 *hexPoints = 0;
    for(int32 i = 7; i > 0; i--)
    {
        v2 point = PointyHexCorner(Vec2(), HEX_SIZE, i);
        arrput(hexPoints, point);
    } 

    v2 *texCoord = 0;
    for(int32 i = 7; i > 0; i--)
    {
        v2 point = PointyHexCorner(Vec2(HEX_TEXTURE_SIZE/2, HEX_TEXTURE_SIZE/2), HEX_TEXTURE_HEX_SIZE, i);
        point.x /= HEX_TEXTURE_SIZE;
        point.y /= HEX_TEXTURE_SIZE;
        arrput(texCoord, point);
    }

    /////////////////////////// MAP SETUP ////////////////////////////////////
    

    offset_coord offsetUnderMouse = {0};

    game_unit *selectedUnit = 0;


    selectedUnit = (game_unit *)MemAlloc(sizeof(game_unit));
    GetMapTile(map, Offset(5, 5))->unit = selectedUnit;
    selectedUnit->coord = Offset(5, 5);
    selectedUnit->movement = 2;
    selectedUnit->movementLeft = 2;
    
    game_unit **unitsToDraw = 0;


    game_city *city = (game_city *)MemAlloc(sizeof(game_city));
    city->coord = Offset(10, 10);
    city->area = OffsetNeighbors(city->coord);
    city->areaLine = CityAreaLine(city);


    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        offsetUnderMouse = ScreenToOffset(GetScreenToWorld2D(GetMousePosition(), camera));
   
        if(IsKeyPressed(KEY_F)) ToggleFullscreen();
        ///////////////////////////// CAMERA ZOOM /////////////////////////////////
        float wheel = GetMouseWheelMove();
		if (wheel != 0)
		{
			// get the world point that is under the mouse
			Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
			// set the offset_coord to where the mouse is
			camera.offset = GetMousePosition();
			// set the target to match, so that the camera maps the world space point under the cursor to the screen space point under the cursor at any zoom
			camera.target = mouseWorldPos;
			// zoom
			cameraZoomTarget = Clamp(camera.zoom + wheel * cameraZoomSpeed, 1.0f, 10.f);
		}

        camera.zoom = Lerp(camera.zoom, cameraZoomTarget, 0.1f);

        //////////////////////// CAMERA MOVEMENT ////////////////////////////
        if(IsKeyDown(KEY_W))
        {
            camera.target.y -= cameraSpeed; 
        }
        if(IsKeyDown(KEY_S))
        {
            camera.target.y += cameraSpeed; 
        }
        if(IsKeyDown(KEY_A))
        {
            camera.target.x -= cameraSpeed; 
        }
        if(IsKeyDown(KEY_D))
        {
            camera.target.x += cameraSpeed; 
        }

        if(IsMouseButtonDown(1))
        {
            if(selectedUnit)
            {
                if(selectedUnit->path)
                {
                    for(int32 i = 0; i < arrlen(selectedUnit->path); i++)
                    {
                        map_tile *pathTile = GetMapTile(map, selectedUnit->path[i]);
                        pathTile->showPath = false;
                    }

                    arrfree(selectedUnit->path);
                }
                
                selectedUnit->path = FindPath(map, selectedUnit->coord, offsetUnderMouse);
                selectedUnit->moving = false;

                for(int32 i = 0; i < arrlen(selectedUnit->path); i++)
                {
                    map_tile *pathTile = GetMapTile(map, selectedUnit->path[i]);
                    pathTile->showPath = true;
                }

                if(selectedUnit->path && MoveCost(GetMapTile(map, selectedUnit->coord), GetMapTile(map, selectedUnit->path[0])) <= selectedUnit->movementLeft)
                {
                    selectedUnit->moving = true;
                }
            }
        }

        if(IsMouseButtonPressed(0))
        {
            map_tile *tile = GetMapTile(map, offsetUnderMouse);

            if(tile && tile->unit)
            {
                if(selectedUnit && selectedUnit->path)
                {
                    for(int32 i = 0; i < arrlen(selectedUnit->path); i++)
                    {
                        map_tile *pathTile = GetMapTile(map, selectedUnit->path[i]);
                        pathTile->showPath = false;
                    }
                }

                selectedUnit = tile->unit;

                if(selectedUnit->path)
                {
                    for(int32 i = 0; i < arrlen(selectedUnit->path); i++)
                    {
                        map_tile *pathTile = GetMapTile(map, selectedUnit->path[i]);
                        pathTile->showPath = true;
                    }
                }
            }
        }

        if(IsKeyPressed(KEY_R))
        {
            selectedUnit->movementLeft = selectedUnit->movement;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(PINK);
            BeginMode2D(camera);

            ////////////////////// RENDERING TILES AND THING ON THEM //////////////////////////////////

            int32 heightToDraw = screenHeight / HEX_CENTRE_DIST_VERT + 3;
            int32 widthToDraw = screenWidth / HEX_CENTRE_DIST_HOR + 4;
            v2 leftTopPixel = Vector2Subtract(camera.target, camera.offset);
            offset_coord coordStart = ScreenToOffset(leftTopPixel);
            coordStart.col -=1;
            if(coordStart.row > 0) coordStart.row -= 1;
            offset_coord coordEnd = coordStart;
            coordEnd.col += widthToDraw;
            coordEnd.row += heightToDraw;            
            
            for(int32 y = coordStart.row; y < coordEnd.row; y++)
            {
                for(int32 x = coordStart.col; x < coordEnd.col; x++)
                {   
                    int32 maxX = map->wrap ? INT32_MAX : map->width;
                    int32 minX = map->wrap ? INT32_MIN : 0;
                    offset_coord coord = Offset(Clamp(x, minX, maxX), Clamp(y, 0, map->height));

                    map_tile *tile = GetMapTile(map, coord);

                    const char *str = TextFormat("%d:%d", coord.row, coord.col);

                    v2 center = OffsetToScreen(coord);
                    Assert(tile);
                    Assert(tile->texture.id >= 0);
                    DrawTexturePoly(tile->texture, center, hexPoints, texCoord, 7, tile->overlayColor);
                    //DrawTexturePoly(treesTex, center, hexPoints, texCoord, 7, WHITE);
                    //DrawPoly(center, 6, HEX_SIZE, 0.0f, tile->color);
                    DrawPolyLines(center, 6, HEX_SIZE, 0.0f, BLACK);
                    DrawText(str, center.x - 10, center.y, 10, BLACK);

                    if(tile->unit)
                    {
                        arrput(unitsToDraw, tile->unit);
                    }

                    if(tile->showPath)
                    {
                        DrawCircleV(center, 30.0f, Fade(BLACK, 0.3f));
                    }
                }
            }

            //////////////////// RENDERING UNITS //////////////////////////////////
            for(int32 i = 0; i < arrlen(unitsToDraw); i++)
            {
                game_unit *unit = unitsToDraw[i];
                v2 drawPos = RealOffsetToScreen(unit->coord, camera, map->width);

                if(arrlen(unit->path) > 0 && unit->moving)
                {
                    bool goNext = false;
                    unit->transition += 0.01f;
                    if(unit->transition > 1.0f)
                    {
                        unit->transition = 1.0f;
                        goNext = true;
                    }

                    offset_coord next = unit->path[0];

                    v2 nextPos = RealOffsetToScreen(next, camera, map->width);

                    drawPos = Vector2Lerp(drawPos, nextPos, unit->transition);

                    if(goNext)
                    {
                        unit->movementLeft -= MoveCost(GetMapTile(map, unit->coord), GetMapTile(map, next));
                        GetMapTile(map, unit->coord)->unit = 0;
                        unit->coord = unit->path[0];
                        unit->transition = 0.0f;
                        GetMapTile(map, next)->showPath = false;
                        GetMapTile(map, next)->unit = unit;
                        arrdel(unit->path, 0);

                        if(arrlen(unit->path) > 0)
                        {
                            if(MoveCost(GetMapTile(map, unit->coord), GetMapTile(map, unit->path[0])) > unit->movementLeft)
                            {
                                unit->moving = false;
                            }
                        }
                        else
                        {
                            unit->moving = false;
                        }
                    }
                }

                DrawRectangleV(drawPos, Vec2(20.0f, 35.0f), GREEN);
            }
            arrfree(unitsToDraw);

            ////////////// RENDERING CITIES ////////////////
            v2 cityDrawPos = RealOffsetToScreen(city->coord, camera, map->width);
            DrawCircleV(cityDrawPos, 30.0f, BLUE);
            for(int32 i = 0; i < arrlen(city->areaLine); i+=2)
            {
                v2 linestart = Vector2Add(city->areaLine[i], cityDrawPos);
                v2 lineEnd = Vector2Add(city->areaLine[i+1], cityDrawPos);
                DrawLineEx(linestart, lineEnd, 3.0f, BLUE);
                
            }

            ////// hex under mouse
            v2 pos = OffsetToScreen(offsetUnderMouse);

            DrawPoly(pos, 6, HEX_SIZE, 0.0f, BLUE);        

            EndMode2D();

            DrawFPS(10, 10);            

        EndDrawing();

        
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}