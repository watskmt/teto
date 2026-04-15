#include "DxLib.h"
#define WIDTH 10
#define HEIGHT 10
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 800

// Point 定義
struct Point
{
    int x;
    int y;
};
Point ConvertTopLeftToBottomLeft(Point p);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ChangeWindowMode(TRUE);
    SetGraphMode(640, 480, 32);
    SetMainWindowText(L"DXLib Project");

    if (DxLib_Init() == -1)
    {
        return -1;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        ClearDrawScreen();

        DrawString(10, 10, L"Hello, DXLib!", GetColor(255, 255, 255));

        ScreenFlip();
    }

    DxLib_End();
    return 0;
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
