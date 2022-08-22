

int guiLabelFontSize = 8;
int guiInnerFontSize = 6;

// used just for dropdowns
bool guiLocked = false;


bool GUIButton(Rectangle rec, const char *text)
{
    bool result = false;

    DrawRectangleRec(rec, BLACK);
    DrawRectangleLinesEx(rec, 2.0f, RED);
    float x = rec.x + ((rec.width - MeasureText(text, guiInnerFontSize)) / 2);
    float y = rec.y + ((rec.height - guiInnerFontSize) / 2.0f);
    DrawText(text, x, y, guiInnerFontSize, WHITE);
    
    result = (CheckCollisionPointRec(GetMousePosition(), rec) && IsMouseButtonPressed(0));

    if(guiLocked) result = false;

    return result;
}

bool GUISlider(Rectangle rec, const char *label, float *val, float min, float max, bool *editMode)
{
    bool result = false;

    Rectangle sliderRec = rec;
    if(strlen(label) > 0)
    {
        sliderRec = (Rectangle){rec.x, rec.y + guiLabelFontSize+1, rec.width, rec.height - guiLabelFontSize-1};
    }

    DrawText(label, rec.x, rec.y, guiLabelFontSize, BLACK);

    DrawRectangleRec(sliderRec, WHITE);
    DrawRectangleLinesEx(sliderRec, 1.0f, BLACK);
    
    if(*editMode && IsMouseButtonUp(0))
    { 
        *editMode = false;
        result = true;
    }
    if(*editMode && IsMouseButtonDown(0))
    {
        *val += (GetMouseDelta().x / sliderRec.width) * (max - min);
        result = true;
    }

    *val = Clamp(*val, min, max);

    float handlePosX = sliderRec.x + ((- min + *val) * (sliderRec.width / (max - min))); //sliderRec.x + (*val * sliderRec.width) / max - min;
    Rectangle handleRec = (Rectangle){handlePosX, sliderRec.y, 2, sliderRec.height};
    DrawRectangleRec(handleRec, BLACK);

    const char *text = TextFormat("%f", *val);
    int length = MeasureText(text, guiInnerFontSize);
    int x = sliderRec.x + (sliderRec.width - length) / 2;
    int y = sliderRec.y + (sliderRec.height - guiInnerFontSize) / 2;
    DrawText(text, x, y, guiInnerFontSize, BLACK);

    bool clicked = CheckCollisionPointRec(GetMousePosition(), sliderRec) && IsMouseButtonPressed(0) && !guiLocked;

    if(clicked)
    {
        *editMode = true;
        *val = ((GetMousePosition().x - sliderRec.x) / sliderRec.width) * (max - min) + min;// ((GetMousePosition().x - sliderRec.x) * max) / sliderRec.width;
        result = true;
    }

    return result;
}

bool GUISpinner(Rectangle rec, const char *label, int *val, int min, int max, bool *editMode)
{
    bool result = false;

    Rectangle spinnerRec = rec;
    if(label && strlen(label) > 0)
    {
        spinnerRec = (Rectangle){rec.x, rec.y + guiLabelFontSize+1, rec.width, rec.height - guiLabelFontSize-1};
    }

    DrawText(label, rec.x, rec.y, guiLabelFontSize, BLACK);

    Rectangle leftButton = (Rectangle){spinnerRec.x, spinnerRec.y, spinnerRec.width * 0.1f, spinnerRec.height};
    Rectangle center = (Rectangle){leftButton.x + leftButton.width + 1, spinnerRec.y, spinnerRec.width - leftButton.width * 2 - 2, spinnerRec.height};
    Rectangle rightButton = (Rectangle){spinnerRec.x + leftButton.width + center.width + 2, spinnerRec.y, leftButton.width, spinnerRec.height};

    DrawRectangleRec(leftButton, WHITE);
    DrawRectangleLinesEx(leftButton, 1.0f, BLACK);
    DrawRectangleRec(center, WHITE);
    DrawRectangleLinesEx(center, 1.0f, BLACK);
    DrawRectangleRec(rightButton, WHITE);
    DrawRectangleLinesEx(rightButton, 1.0f, BLACK);

    int signWidth = MeasureText("<", guiInnerFontSize);
    DrawText("<", leftButton.x + (leftButton.width - signWidth) / 2, leftButton.y + (leftButton.height - guiInnerFontSize) / 2, guiInnerFontSize, BLACK);
    DrawText(">", rightButton.x + (rightButton.width - signWidth) / 2, rightButton.y + (rightButton.height - guiInnerFontSize) / 2, guiInnerFontSize, BLACK);

    bool clickedLeft = CheckCollisionPointRec(GetMousePosition(), leftButton) && IsMouseButtonPressed(0);
    bool clickedRight = CheckCollisionPointRec(GetMousePosition(), rightButton) && IsMouseButtonPressed(0);
    bool clickedCenter = CheckCollisionPointRec(GetMousePosition(), center) && IsMouseButtonPressed(0);

    if(*editMode && IsMouseButtonPressed(0) && !clickedCenter)
    {
        *editMode = false;
        result =true;
    }

    if(*editMode)
    {
        char number[10] = {0};
        int index = 0;
        const char *currentNum = TextFormat("%d", *val);
        for(int i = 0; i < strlen(currentNum); i++)
        {
            number[i] = currentNum[i];
            index++;
        }
        int key;

        if(IsKeyPressed(KEY_BACKSPACE) && index > 0)
        {
            number[index-1] = '\0';
        }

        while((key = GetKeyPressed()) && index < 10)
        {
            switch (key)
            {
            case KEY_KP_1:
            case KEY_ONE:
                number[index] = '1';
                break;
            case KEY_KP_2:
            case KEY_TWO:
                number[index] = '2';
                break;
            case KEY_KP_3:
            case KEY_THREE:
                number[index] = '3';
                break;
            case KEY_KP_4:
            case KEY_FOUR:
                number[index] = '4';
                break;
            case KEY_KP_5:
            case KEY_FIVE:
                number[index] = '5';
                break;
            case KEY_KP_6:
            case KEY_SIX:
                number[index] = '6';
                break;
            case KEY_KP_7:
            case KEY_SEVEN:
                number[index] = '7';
                break;
            case KEY_KP_8:
            case KEY_EIGHT:
                number[index] = '8';
                break;
            case KEY_KP_9:
            case KEY_NINE:
                number[index] = '9';
                break;
            case KEY_KP_0:
            case KEY_ZERO:
                number[index] = '0';
                break;
            
            default:
                break;
            }

            index++;
        } 
        
        if(index > 0)
        {
            char * num = (char *)calloc(index + 1, sizeof(char));
            for(int i = 0; i < index; i++)
            {
                num[i] = number[i];
            }
            num[index] = '\0';
            *val = atoi(num);
        }
    }

    if(clickedLeft && !guiLocked)
    {
        *val = Clamp((*val) - 1, min, max);
        result = true;
    }
    else if(clickedRight && !guiLocked)
    {
        *val = Clamp((*val) + 1, min, max);
        result = true;
    }
    else if(clickedCenter && !guiLocked)
    {
        *editMode = true;
    }

    const char * valText = TextFormat("%i", *val);
    int valWidth = MeasureText(valText, guiInnerFontSize);
    if(*editMode)
    {
        DrawRectangle(center.x + (center.width - valWidth) / 2 + valWidth, center.y + (center.height - guiInnerFontSize) / 2,  4, guiInnerFontSize + 1, BLUE);
    }
    DrawText(valText, center.x + (center.width - valWidth) / 2, center.y + (center.height - guiInnerFontSize) / 2, guiInnerFontSize, BLACK);

    return result;
}

bool GUIDropDown(Rectangle rec, const char* label, unsigned int *val, const char *values, bool *editMode)
{
    bool result = false;

    Rectangle mainRec = rec;
    if(strlen(label) > 0)
    {
        mainRec = (Rectangle){rec.x, rec.y + guiLabelFontSize+1, rec.width, rec.height - guiLabelFontSize-1};
    }
    DrawText(label, rec.x, rec.y, guiLabelFontSize, BLACK);

    int valueCount = 0;
    const char **splitValues = TextSplit(values, ';', &valueCount);

    DrawRectangleRec(mainRec, WHITE);
    DrawRectangleLinesEx(mainRec, 1.0f, BLACK);
    DrawText(splitValues[*val], mainRec.x + (mainRec.width - MeasureText(splitValues[*val], guiInnerFontSize)) / 2, mainRec.y + (mainRec.height - guiInnerFontSize) / 2, guiInnerFontSize, BLACK);

    bool clicked = CheckCollisionPointRec(GetMousePosition(), mainRec) && IsMouseButtonPressed(0);

    if(clicked) 
    {
        if(*editMode)
        {
            *editMode = false;
            guiLocked = false;

        }
        else if(!guiLocked)
        {
            *editMode = true;
            guiLocked = true;
        }
    }

    if(*editMode)
    {
        for(int i = 0; i < valueCount; i++)
        {
            Rectangle r = (Rectangle){mainRec.x, mainRec.y + mainRec.height * (i + 1), mainRec.width, mainRec.height};
            DrawRectangleRec(r, WHITE);
            if(CheckCollisionPointRec(GetMousePosition(), r))
            {
                DrawRectangleRec(r, Fade(BLUE, 0.5f));

                if(IsMouseButtonPressed(0))
                {
                    *val = i;
                    *editMode = false;
                    guiLocked = false;
                    result = true;
                }
            }
            DrawText(splitValues[i], mainRec.x + (mainRec.width - MeasureText(splitValues[*val], guiInnerFontSize)) / 2, mainRec.y + mainRec.height * (i + 1) + (mainRec.height - guiInnerFontSize) / 2, guiInnerFontSize, BLACK);
        }

        DrawLine(mainRec.x, mainRec.y + mainRec.height, mainRec.x, mainRec.y + mainRec.height * (valueCount + 1), BLACK);
        DrawLine(mainRec.x + mainRec.width, mainRec.y + mainRec.height, mainRec.x + mainRec.width, mainRec.y + mainRec.height * (valueCount + 1), BLACK);
        DrawLine(mainRec.x, mainRec.y + mainRec.height * (valueCount + 1), mainRec.x + mainRec.width, mainRec.y + mainRec.height * (valueCount + 1), BLACK);
    }

    return result;
}


