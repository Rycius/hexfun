#define MY_DEBUG 1

#include "main.h"
#include "gui.cpp"


#define SQRT3 1.73205080757f
#define HEX_SIZE 40.0f
#define HEX_HEIGHT (HEX_SIZE * 2.0f)
#define HEX_WIDTH (SQRT3 * HEX_SIZE)
#define HEX_CENTRE_DIST_VERT (0.75f * HEX_HEIGHT)
#define HEX_CENTRE_DIST_HOR HEX_WIDTH

#define HEX_TEXTURE_SIZE 512
#define HEX_TEXTURE_HEX_SIZE (HEX_TEXTURE_SIZE / 2)

v2 PointyHexCorner(v2 center, float size, int32 i)
{
    v2 result = Vec2();

    float angleDeg = 60.0f * i - 30.0f;
    float angleRad = PI / 180 * angleDeg;

    result.x = center.x + size * cosf(angleRad);
    result.y = center.y + size * sinf(angleRad);

    return result;
}

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

inline axial_coord OffsetToAxial(offset_coord o)
{
    axial_coord result = {0};

    result.q = o.col - (o.row - (o.row & 1)) / 2;
    result.r = o.row;

    return result;
}

inline offset_coord CubeToOffset(cube_coord cube)
{
    offset_coord result = {0};

    result.col = cube.q + (cube.r - (cube.r & 1)) / 2;
    result.row = cube.r;

    return result;
}

inline cube_coord offset_coordToCube(offset_coord o)
{
    cube_coord result = {0};

    result.q = o.col - (o.row - (o.row & 1)) / 2;
    result.r = o.row;
    result.s = -result.q - result.r;

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

inline offset_coord ScreenToOffset(v2 pos)
{
    offset_coord result = {0};

    result = AxialToOffset(ScreenToAxial(pos));

    return result;
}

inline v2 AxialToScreen(axial_coord axial)
{
    v2 result = {0};

    result.x = HEX_SIZE * (SQRT3 * axial.q + SQRT3 / 2.0f * axial.r);
    result.y = HEX_SIZE * (3.0f / 2.0f * axial.r);

    return result;
}

inline v2 OffsetToScreen(offset_coord o)
{
    v2 result = {0};

    result.x = HEX_SIZE * SQRT3 * (o.col + 0.5f * (o.row&1));
    result.y = HEX_SIZE * 3.0f / 2.0f * o.row;

    return result;
}

bool IsOnScreen(v2 pos, v2 dim, Camera2D camera)
{
    bool result = false;

    v2 zoomDim = Vector2Scale(dim, camera.zoom);
    v2 screenPos = GetWorldToScreen2D(pos, camera);
    Rectangle screenRec = Rec(screenPos.x - zoomDim.x/2, screenPos.y - zoomDim.y/2, zoomDim.x, zoomDim.y);
    
    result = CheckCollisionRecs(screenRec, Rec(0, 0, GetScreenWidth(), GetScreenHeight()));

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

map_tile *GetMapTile(map_tile *map, int32 mapWidth, int32 mapHeight, offset_coord offset)
{
    map_tile *result = 0;

    Assert(offset.row >= 0 && offset.row < mapHeight);

    uint32 address = offset.row * mapWidth;
    if(offset.col < 0)
        address += mapWidth + offset.col % mapWidth;
    else
        address += offset.col % mapWidth;

    result = &map[address];

    return result;
}

int main() 
{

    Image img = GenImageColor(HEX_TEXTURE_SIZE, HEX_TEXTURE_SIZE, Fade(WHITE, 0.0f));

    v2 prevPoint = PointyHexCorner(Vec2(HEX_TEXTURE_SIZE/2, HEX_TEXTURE_SIZE/2), HEX_TEXTURE_HEX_SIZE, 0);
    for(int32 i = 1; i < 6; i++)
    {
        v2 point = PointyHexCorner(Vec2(HEX_TEXTURE_SIZE/2, HEX_TEXTURE_SIZE/2), HEX_TEXTURE_HEX_SIZE, i);
        ImageDrawLineV(&img, prevPoint, point, BLACK); 
        prevPoint = point;
    } 
    ImageDrawLineV(&img, prevPoint, PointyHexCorner(Vec2(HEX_TEXTURE_SIZE/2, HEX_TEXTURE_SIZE/2), HEX_TEXTURE_HEX_SIZE, 0), BLACK); 
    ExportImage(img, "img.png");

    // Initialization
    //--------------------------------------------------------------------------------------
    const int32 screenWidth = 1920;
    const int32 screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "raylib");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second


    Camera2D camera = {0};
    camera.zoom = 3.0f;
    camera.offset.x = 0.0f; 
    camera.offset.y = 0.0f; 

    float cameraSpeed = 10.0f;
    float cameraZoomSpeed = 2.0f;


    int32 mapWidth = 74;
    int32 mapHeight = 46;

    Image hexGrasslandImg = LoadImage("../resources/hex_grassland.png");
    Texture hexGrasslandTex = LoadTextureFromImage(hexGrasslandImg);

    Image hexDesertImg = LoadImage("../resources/hex_Desert.png");
    Texture hexDesertTex = LoadTextureFromImage(hexDesertImg);

    Image treesImg = LoadImage("../resources/trees.png");
    Texture treesTex = LoadTextureFromImage(treesImg);

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
    map_tile *map = 0;
    arrsetcap(map, mapWidth * mapHeight);

    for(int32 y = 0; y < mapHeight; y++)
    {
        for(int32 x = 0; x < mapWidth; x++)
        {
            map_tile tile = {0};

            tile.offset.col = x;
            tile.offset.row = y;

            tile.texture = hexGrasslandTex;
            if(y > 5 && y < 10 && x > 3 && x < 10)
            {
                tile.texture = hexDesertTex;
            }

            tile.overlayColor = WHITE;
            if(x == 0) 
                tile.overlayColor = BLUE;
            else if(x == mapWidth - 1)
                tile.overlayColor = RED;

            arrput(map, tile);
        }
    }

    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
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
			camera.zoom = Clamp(camera.zoom + wheel * cameraZoomSpeed, 1.0f, 10.f);
			
		}

        //////////////////////// CAMERA MOVEMENT ////////////////////////////
        if(IsKeyDown(KEY_W))
        {
            camera.target.y -= cameraSpeed; 
            TraceLog(LOG_INFO, TextFormat("OFFSET x:%.2f y:%.2f", camera.offset.x, camera.offset.y));
            TraceLog(LOG_INFO, TextFormat("TARGET x:%.2f y:%.2f", camera.target.x, camera.target.y));
        }
        if(IsKeyDown(KEY_S))
        {
            camera.target.y += cameraSpeed; 
            TraceLog(LOG_INFO, TextFormat("OFFSET x:%.2f y:%.2f", camera.offset.x, camera.offset.y));
            TraceLog(LOG_INFO, TextFormat("TARGET x:%.2f y:%.2f", camera.target.x, camera.target.y));
        }
        if(IsKeyDown(KEY_A))
        {
            camera.target.x -= cameraSpeed; 
            TraceLog(LOG_INFO, TextFormat("OFFSET x:%.2f y:%.2f", camera.offset.x, camera.offset.y));
            TraceLog(LOG_INFO, TextFormat("TARGET x:%.2f y:%.2f", camera.target.x, camera.target.y));
        }
        if(IsKeyDown(KEY_D))
        {
            camera.target.x += cameraSpeed; 
            TraceLog(LOG_INFO, TextFormat("OFFSET x:%.2f y:%.2f", camera.offset.x, camera.offset.y));
            TraceLog(LOG_INFO, TextFormat("TARGET x:%.2f y:%.2f",   camera.target.x, camera.target.y));
        }

        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            BeginMode2D(camera);

            

            int32 heightToDraw = screenHeight / HEX_CENTRE_DIST_VERT + 3;
            int32 widthToDraw = screenWidth / HEX_CENTRE_DIST_HOR + 3;
           
            v2 leftTopPixel = Vector2Subtract(camera.target, camera.offset);
            offset_coord offsetStart = AxialToOffset(ScreenToAxial(leftTopPixel));
            offsetStart.col -=1;
            if(offsetStart.row > 0) offsetStart.row -= 1;
            

            for(int32 y = offsetStart.row; y < offsetStart.row + heightToDraw; y++)
            {
                for(int32 x = offsetStart.col; x < offsetStart.col + widthToDraw; x++)
                {
                    offset_coord offset = Offset(x, Clamp(y, 0, mapHeight-1));

                    map_tile *tile = GetMapTile(map, mapWidth, mapHeight, offset);

                    const char *str = TextFormat("%d:%d", tile->offset.row, tile->offset.col);

                    v2 center = OffsetToScreen(offset);
                    
                    DrawTexturePoly(tile->texture, center, hexPoints, texCoord, 7, tile->overlayColor);
                    DrawTexturePoly(treesTex, center, hexPoints, texCoord, 7, WHITE);
                    DrawTexturePoly(treesTex, center, hexPoints, texCoord, 7, WHITE);
                    //DrawPoly(center, 6, HEX_SIZE, 0.0f, tile->color);
                    DrawPolyLines(center, 6, HEX_SIZE, 0.0f, BLACK);
                    DrawText(str, center.x - 10, center.y, 10, BLACK);
                    
                }
            }

            ////// hex under mouse
            offset_coord offset = ScreenToOffset(GetScreenToWorld2D(GetMousePosition(), camera));
            v2 pos = OffsetToScreen(offset);

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