#include "DxLib.h"
#define WIDTH 10
#define HEIGHT 10
#define CELL_SIZE 40
#define MARGIN_X 100
#define MARGIN_Y 10
#define WINDOW_WIDTH CELL_SIZE * WIDTH+MARGIN_X * 2
#define WINDOW_HEIGHT CELL_SIZE * HEIGHT+MARGIN_Y * 2

typedef struct Point // 座標用の構造体
{
    int x;
    int y;
}Point;

typedef struct Block // ブロックの構造体名
{
    int x;
    int y;
    int type;
    int rotation;
}Block;

// 左上原点 → 左下原点に変換する関数 
Point ConvertTopLeftToBottomLeft(Point p);

void initializeDrawScreen(void);
void DrawCell(int x, int y);
Point GameBoardToScreen(Point p);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int cellSizeX = WINDOW_WIDTH / WIDTH;
    int cellSizeY = WINDOW_HEIGHT / HEIGHT;
    ChangeWindowMode(TRUE);
    SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);
    SetMainWindowText(L"DXLib Project");

    if (DxLib_Init() == -1)
    {
        return -1;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        ClearDrawScreen();

		initializeDrawScreen();

        ScreenFlip();
    }

    DxLib_End();
    return 0;
}

void initializeDrawScreen(void)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            DrawCell(x, y);
        }
    }
}

// DrawCell: 指定した盤面座標のセルを画面に描画する。
//    - 入力: Point p (x, y は左下基準のセル番号)
//    - 出力: なし
void DrawCell(int x, int y)
{
    Point start = {x, y};
	Point end = { x + 1, y + 1 };
    Point converted_start = GameBoardToScreen(start);
    Point converted_end = GameBoardToScreen(end);

    DrawBox(converted_start.x, converted_start.y, converted_end.x - 1, converted_end.y - 1, GetColor(255, 255, 255), FALSE);
}


// 左上原点 (0,0 が左上) を左下原点 (0,0 が左下) に変換する関数を定義する。
//    - 入力: Point p (x, y は左上原点座標)
//    - 出力: Point r (x は変化なし、y は上下反転された値)
//    - 変換式: r.y = (WINDOW_HEIGHT_ - 1) - p.y
// 左上原点 -> 左下原点 に変換する関数

Point ConvertTopLeftToBottomLeft(Point p)
{
    Point r;
    r.x = p.x;
    // 上下反転：描画領域の高さが WINDOW_HEIGHT の場合、y = 0 は最上部、
    // 反転後は WINDOW_HEIGHT-1
    r.y = (WINDOW_HEIGHT - 1) - p.y;
    return r;
}

// 盤面座標（セル座標） -> スクリーン座標 に変換する関数
//    入力：Point p　セル座標
// 　　出力：そのセルの左下角のスクリーン座標
Point GameBoardToScreen(Point p)
{
    Point r;
    r.x = p.x * CELL_SIZE + MARGIN_X;
    r.y = p.y * CELL_SIZE + MARGIN_Y;

	r = ConvertTopLeftToBottomLeft(r);
    return r;
}
