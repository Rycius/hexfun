
enum gui_element_type
{
    GUI_BUTTON,
    GUI_LABEL,
};

struct gui_element
{
    gui_element_type type;
    Rectangle rec;
    const char *text;
    v2 textPos;
    Color color;
    Color borderColor;
    Color textColor;
    bool hot;
    bool clicked;
};

Font gameFont;
const float gameFontSize = 24.0f;
const float gameTextSpacing = 1.0f;

v2 _nextElm = Vec2();
v2 _parentDim = Vec2();
float _elmPadding = 10.0f;


void GUISetup()
{
    gameFont = GetFontDefault();
}

void GUIVertLayoutBegin(v2 start, v2 dim)
{
    _nextElm = start;
    _parentDim = dim;
}

gui_element GUILabel(const char *text, Rectangle rec = (Rectangle){0, 0, 0, 0})
{
    gui_element result = {};
        result.type = GUI_LABEL;
        result.rec = rec;
        if((rec.x == 0 && rec.y == 0))
        {
            result.rec = Rec(_nextElm.x, _nextElm.y, _parentDim.x, _parentDim.y);
            _nextElm.y += result.rec.height + _elmPadding;
        }
        result.text = text;

        v2 textDim = MeasureTextEx(gameFont, text, gameFontSize, gameTextSpacing);
        result.textPos.x = result.rec.x + (result.rec.width - textDim.x) / 2.0f;
        result.textPos.y = result.rec.y + (result.rec.height - textDim.y) / 2.0f;

        result.color = WHITE;
        result.borderColor = BLACK;
        result.textColor = BLACK;

    return result;
}

gui_element GUIButton(const char *text, Rectangle rec = (Rectangle){0, 0, 0, 0})
{
    gui_element result = {};
        result.type = GUI_BUTTON;
        if((rec.x == 0 && rec.y == 0))
        {
            result.rec = Rec(_nextElm.x, _nextElm.y, _parentDim.x, _parentDim.y);
            _nextElm.y += result.rec.height + _elmPadding;
        }
        result.text = text;

        v2 textDim = MeasureTextEx(gameFont, text, gameFontSize, gameTextSpacing);
        result.textPos.x = result.rec.x + (result.rec.width - textDim.x) / 2.0f;
        result.textPos.y = result.rec.y + (result.rec.height - textDim.y) / 2.0f;

        result.color = WHITE;
        result.borderColor = BLACK;
        result.textColor = BLACK;

    return result;
}

bool GUIProcessElement(gui_element elm)
{
    bool result = false;

    bool hot = CheckCollisionPointRec(GetMousePosition(), elm.rec);
    bool clicked = (hot && IsMouseButtonPressed(0));

    switch (elm.type)
    {
        case GUI_LABEL:
        {
            elm.hot = hot;
        } break;
        case GUI_BUTTON:
        {
            elm.hot = hot;
            if(clicked)
            {
                elm.clicked = clicked;
                result = true;
            }
        } break;
        default: 
            break;
    }

    return result;
}


void GUIDrawElement(gui_element elm)
{
    switch (elm.type)
    {
        case GUI_LABEL:
        {
            DrawRectanglePro(elm.rec, Vec2(), 0.0f, elm.color);
            DrawRectangleLinesEx(elm.rec, 2.0f, elm.borderColor);
            DrawTextPro(gameFont, elm.text, elm.textPos, Vec2(), 0.0f, gameFontSize, gameTextSpacing, elm.textColor);
        } break;
        case GUI_BUTTON:
        {
            DrawRectanglePro(elm.rec, Vec2(), 0.0f, elm.color);
            DrawRectangleLinesEx(elm.rec, 2.0f, elm.borderColor);
            DrawTextPro(gameFont, elm.text, elm.textPos, Vec2(), 0.0f, gameFontSize, gameTextSpacing, elm.textColor);
        } break;
        default:
            break;
    }
}