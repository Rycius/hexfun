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

inline v2 *HexEdgeLine(v2 center, direction dir)
{
    v2 *result = 0;

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
        v2 offset = Vector2Subtract(OffsetToScreen(city->area[i]), OffsetToScreen(city->coord));
        offset_coord *neighbors = OffsetNeighbors(city->area[i]);

        for(int32 n = 0; n < arrlen(neighbors); n++)
        {
            if(neighbors[n].col == city->coord.col && neighbors[n].row == city->coord.row) continue;
            if(ArrContainsOffset(city->area, neighbors[n])) continue;

            v2 *line = HexEdgeLine(offset, (direction)n);
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


void AddUnit(game_player *player, offset_coord coord)
{
    map_tile *tile = GetMapTile(coord);
    if(tile->unit != 0)
    {
        TraceLog(LOG_INFO, "Trying to add a unit on the tile where one already is. Player: %s", player->name);
        return;
    } 

    if(arrlen(player->units) >= UNITS_PER_PLAYER_CAP - 1)
    {
        TraceLog(LOG_INFO, "Unit cap reached. Player: %s", player->name);
        return;
    }

    game_unit unit = {0};
    unit.coord = coord;
    unit.type = UNIT_TYPE_WARRIOR;
    unit.rec = Rec(Vec2(), Vec2(HEX_WIDTH*0.7f, HEX_HEIGHT*0.7f));
    unit.movement = 2;
    unit.movementLeft = 2;
    arrpush(player->units, unit);
    
    tile->unit = &player->units[arrlen(player->units)-1];
}

bool CanPlaceCity(offset_coord coord)
{
    bool result = true;

    map_tile *tile = GetMapTile(coord);

    if(tile->city) result = false;
    else result = tile->passable;

    return result;
}

bool PlaceCity(game_player *player, offset_coord coord)
{
    bool result = false;

    if(CanPlaceCity(coord))
    {
        if(arrlen(player->cities) >= CITIES_PER_PLAYER_CAP)
        {
            TraceLog(LOG_INFO, "Cities cap reached. Player: %s", player->name);
            return false;
        }

        game_city city = {0};
        city.coord = coord;
        city.area = OffsetNeighbors(coord);
        city.areaLine = CityAreaLine(&city);
        city.owner = player;

        arrpush(player->cities, city);
        GetMapTile(coord)->city = &player->cities[arrlen(player->cities)-1];
        result = true;
    }

    return result;
}

void AddTeritory(game_city *city, offset_coord coord)
{
    arrpush(city->area, coord);
    arrfree(city->areaLine);
    city->areaLine = CityAreaLine(city);
}


void AddPlayer(game_data *game, const char *name)
{
    game_player player = {0};
    player.color = YELLOW; // TODO: needs to get it from list, different for every player
    player.name = name;

    arrpush(game->players, player);

    arrsetcap(game->players[arrlen(game->players)-1].cities, CITIES_PER_PLAYER_CAP);
    arrsetcap(game->players[arrlen(game->players)-1].units, UNITS_PER_PLAYER_CAP);
}


game_data *CreateGame()
{
    game_data *result = (game_data *)MemAlloc(sizeof(game_data));

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
    float cameraZoomStep = 2.0f;
    float cameraZoomSpeed = 0.05f;
    float cameraMaxZoom = 5.0f;
    float cameraZoomTarget = 1.0f;
    

    Texture2D hexGrasslandTex = LoadTexture("../resources/hex_grassland.png");
    Texture2D hexDesertTex = LoadTexture("../resources/hex_Desert.png");
    Texture2D treesTex = LoadTexture("../resources/trees.png");
    Texture2D warriorTex = LoadTexture("../resources/warrior.png");

    Image hexImg = GenImageColor(32, 32, WHITE);
    Texture2D hexTex = LoadTextureFromImage(hexImg);
    UnloadImage(hexImg);


    InitMap(44, 26, true);
    GenerateTerrain();
    

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

    /////////////////////////// GAME SETUP ////////////////////////////////////

    game_data *game = CreateGame();

    AddPlayer(game, TextFormat("Player1"));

    AddUnit(game->players, Offset(5, 5));
    PlaceCity(game->players, Offset(10, 10));



    game_unit *selectedUnit = 0;

    game_unit **unitsToDraw = 0;
    game_city **citiesToDraw = 0;
    
    offset_coord offsetUnderMouse = {0};


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
			cameraZoomTarget = Clamp(camera.zoom + wheel * cameraZoomStep, 1.0f, cameraMaxZoom);
		}

        camera.zoom = Lerp(camera.zoom, cameraZoomTarget, cameraZoomSpeed);

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
                        map_tile *pathTile = GetMapTile(selectedUnit->path[i]);
                        pathTile->showPath = false;
                    }

                    arrfree(selectedUnit->path);
                }
                
                selectedUnit->path = FindPath(selectedUnit->coord, offsetUnderMouse);
                selectedUnit->moving = false;

                for(int32 i = 0; i < arrlen(selectedUnit->path); i++)
                {
                    map_tile *pathTile = GetMapTile(selectedUnit->path[i]);
                    pathTile->showPath = true;
                }

                if(selectedUnit->path && MoveCost(GetMapTile(selectedUnit->coord), GetMapTile(selectedUnit->path[0])) <= selectedUnit->movementLeft)
                {
                    selectedUnit->moving = true;
                }
            }
        }

        if(IsMouseButtonPressed(0))
        {
            map_tile *tile = GetMapTile(offsetUnderMouse);

            if(tile && tile->unit)
            {
                if(selectedUnit && selectedUnit->path)
                {
                    for(int32 i = 0; i < arrlen(selectedUnit->path); i++)
                    {
                        map_tile *pathTile = GetMapTile(selectedUnit->path[i]);
                        pathTile->showPath = false;
                    }
                }

                selectedUnit = tile->unit;

                if(selectedUnit->path)
                {
                    for(int32 i = 0; i < arrlen(selectedUnit->path); i++)
                    {
                        map_tile *pathTile = GetMapTile(selectedUnit->path[i]);
                        pathTile->showPath = true;
                    }
                }
            }
            else
            {
                selectedUnit = 0;
            }
        }

        if(IsKeyPressed(KEY_R))
        {
            //selectedUnit->movementLeft = selectedUnit->movement;
            //AddTeritory(game->players->cities, Offset(10, 12));
            //AddTeritory(game->players->cities, Offset(9, 13));

            offset_coord *ring = GetRing(game->players->cities->coord, 2);

            for(int32 i = 0; i < arrlen(ring); i++)
            {
                AddTeritory(game->players->cities, ring[i]);
            }

            arrfree(ring);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(PINK);
            BeginMode2D(camera);

            ////////////////////// RENDERING TILES AND THINGS ON THEM //////////////////////////////////

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

                    map_tile *tile = GetMapTile(coord);

                    const char *str = TextFormat("%d:%d", coord.row, coord.col);

                    v2 center = OffsetToScreen(coord);
                    Assert(tile);
                    
                    Color tileOverlayColor = WHITE;

                    switch (tile->terrain.type)
                    {
                    case TERRAIN_TYPE_OCEAN:
                        tileOverlayColor = (Color){0, 85, 195, 255};
                        break;
                    case TERRAIN_TYPE_LAKE:
                        tileOverlayColor = (Color){100, 177, 255, 255};
                        break;
                    case TERRAIN_TYPE_COAST:
                        tileOverlayColor = (Color){90, 145, 210, 255};
                    case TERRAIN_TYPE_GRASSLAND:
                        tileOverlayColor = (Color){0, 155, 0, 255};
                        break;
                    case TERRAIN_TYPE_PLAIN:
                        tileOverlayColor = (Color){190, 215, 145, 255};
                        break;
                    case TERRAIN_TYPE_DESERT:
                        tileOverlayColor = (Color){235, 235, 190, 255};
                        break;
                        break;
                    case TERRAIN_TYPE_TUNDRA:
                        tileOverlayColor = (Color){200, 200, 200, 255};
                        break;
                    case TERRAIN_TYPE_SNOW:
                        tileOverlayColor = (Color){225, 225, 225, 255};
                        break;

                    default:
                        break;
                    }

                    switch (tile->terrain.modifier)
                    {
                    case TERRAIN_MODIFIER_HILL:
                        tileOverlayColor = (Color){180, 140, 100, 255};
                        break;
                    case TERRAIN_MODIFIER_MOUNTAIN:
                        tileOverlayColor = RED;
                        break;
                    
                    default:
                        break;
                    }


                    DrawTexturePoly(hexTex, center, hexPoints, texCoord, 7, tileOverlayColor);
                    //DrawTexturePoly(treesTex, center, hexPoints, texCoord, 7, WHITE);
                    //DrawPoly(center, 6, HEX_SIZE, 0.0f, tile->color);
                    DrawPolyLines(center, 6, HEX_SIZE, 0.0f, BLACK);
                    DrawText(str, center.x - 10, center.y, 10, BLACK);

                    if(tile->unit)
                    {
                        arrpush(unitsToDraw, tile->unit);
                    }

                    if(tile->city)
                    {
                        arrpush(citiesToDraw, tile->city);
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
                        unit->movementLeft -= MoveCost(GetMapTile(unit->coord), GetMapTile(next));
                        GetMapTile(unit->coord)->unit = 0;
                        unit->coord = unit->path[0];
                        unit->transition = 0.0f;
                        GetMapTile(next)->showPath = false;
                        GetMapTile(next)->unit = unit;
                        arrdel(unit->path, 0);

                        if(arrlen(unit->path) > 0)
                        {
                            if(MoveCost(GetMapTile(unit->coord), GetMapTile(unit->path[0])) > unit->movementLeft)
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

                Texture2D unitTexture = warriorTex;
                Rectangle sourceRec = Rec(0, 0, warriorTex.width, warriorTex.height);
                Rectangle destRec = Rec(drawPos, RecDim(unit->rec));
                v2 origin = Vector2Scale(RecDim(unit->rec), 0.5f);
                float rotation = 0.0f;
                Color overlay = WHITE;

                DrawTexturePro(unitTexture, sourceRec, destRec, origin, rotation, overlay);
            }
            arrfree(unitsToDraw);

            ////////////// RENDERING CITIES ////////////////
            for(int32 i = 0; i < arrlen(citiesToDraw); i++)
            {
                game_city *city = citiesToDraw[i];
                v2 cityDrawPos = RealOffsetToScreen(city->coord, camera, map->width);
                DrawCircleV(cityDrawPos, 30.0f, BLUE);

                for(int32 k = 0; k < arrlen(city->areaLine); k+=2)
                {
                    v2 linestart = Vector2Add(city->areaLine[k], cityDrawPos);
                    v2 lineEnd = Vector2Add(city->areaLine[k+1], cityDrawPos);
                    DrawLineEx(linestart, lineEnd, 3.0f, city->owner->color);
                }
            }
            arrfree(citiesToDraw);

            ////// hex under mouse
            v2 pos = OffsetToScreen(offsetUnderMouse);

            DrawPolyLinesEx(pos, 6, HEX_SIZE, 0.0f, 2.0f, GOLD);        

            EndMode2D();

            /////////////////////////////// UI DRAWING ///////////////////////////////
            if(selectedUnit)
            {
                float selectedUnitPanelWidth = screenWidth * 0.4f;
                float selectedUnitPanelHeight = screenHeight * 0.2f;
                Rectangle selectedUnitPanelRec = Rec(0, screenHeight - selectedUnitPanelHeight, selectedUnitPanelWidth, selectedUnitPanelHeight);
                v2 selectedUnitPanelOrigin = Vec2();
                float selectedUnitPanelRotation = 0.0f;
                Color selectedUnitPanelColor = GRAY;
                DrawRectanglePro(selectedUnitPanelRec, selectedUnitPanelOrigin, selectedUnitPanelRotation, selectedUnitPanelColor);


                Font unitTypeTextFont = GetFontDefault();
                const char *unitTypeTextText = "WARIOR";
                v2 unitTypeTextPos = Vec2(selectedUnitPanelRec.x + 5.0f, selectedUnitPanelRec.y + 5.0f);
                v2 unitTypeTextOrigin = Vec2();
                float unitTypeTextRotation = 0.0f;
                float unitTypeTextFontSize = 12.0f;
                float unitTypeTextSpacing = 1.0f;
                Color unitTypeTextColor = BLACK;

                DrawTextPro(unitTypeTextFont, unitTypeTextText, unitTypeTextPos, unitTypeTextOrigin, unitTypeTextRotation, unitTypeTextFontSize, unitTypeTextSpacing, unitTypeTextColor);


                Font movementTextFont = GetFontDefault();
                const char *movementTextText = TextFormat("MOVEMENT: %.0f / %.0f", selectedUnit->movementLeft, selectedUnit->movement);
                v2 movementTextPos = Vec2(unitTypeTextPos.x + 10.0f, selectedUnitPanelRec.y + unitTypeTextFontSize * 2.0f);
                v2 movementTextOrigin = Vec2();
                float movementTextRotation = 0.0f;
                float movementTextFontSize = 12.0f;
                float movementTextSpacing = 1.0f;
                Color movementTextColor = BLACK;

                DrawTextPro(movementTextFont, movementTextText, movementTextPos, movementTextOrigin, movementTextRotation, movementTextFontSize, movementTextSpacing, movementTextColor);
            }

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