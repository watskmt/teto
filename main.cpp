#include "DxLib.h"
#include <stdlib.h>
#include <time.h>

#define WIDTH 10
#define HEIGHT 20
#define CELL_SIZE 30
#define MARGIN_X 60
#define MARGIN_Y 10
#define WINDOW_WIDTH (CELL_SIZE * WIDTH + MARGIN_X * 2)
#define WINDOW_HEIGHT (CELL_SIZE * HEIGHT + MARGIN_Y * 2)
#define EMPTY 0


// ===== 構造体 =====
typedef struct Point
{
	int x;
	int y;
} Point;

typedef struct Block
{
	int x;
	int y;
	int type;
	int rotation;
} Block;

int board[HEIGHT][WIDTH];

// ===== ブロック種類 =====
enum
{
	BLOCK_I,
	BLOCK_O,
	BLOCK_T,
	BLOCK_S,
	BLOCK_Z,
	BLOCK_J,
	BLOCK_L
};

// ===== ミノ形状 =====
int mino[7][4][4] =
{
	{ // I
		{0,0,0,0},
		{1,1,1,1},
		{0,0,0,0},
		{0,0,0,0}
	},
	{ // O
		{0,1,1,0},
		{0,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{ // T
		{0,1,0,0},
		{1,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{ // S
		{0,1,1,0},
		{1,1,0,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{ // Z
		{1,1,0,0},
		{0,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{ // J
		{1,0,0,0},
		{1,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{ // L
		{0,0,1,0},
		{1,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	}
};

// ===== 関数宣言 =====
Point ConvertTopLeftToBottomLeft(Point p);
Point GameBoardToScreen(Point p);
void initializeDrawScreen(void);
void DrawCell(int x, int y, int color);
int GetBlockColor(int type);
int CanMove(Block block, int dx);
int isDropTiming(void);
int dropBlock(Block* block);
int moveBlock(Block* block);
int drawBlock(Block block);

// ===== メイン =====
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	srand(time(NULL));
	ChangeWindowMode(TRUE);
	SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);
	if (DxLib_Init() == -1) return -1;
	SetDrawScreen(DX_SCREEN_BACK);

	// テストブロック
	Block block;
	block.x = 3;
	block.y = 18;
	block.type = rand() % 7;
	block.rotation = 0;

	int lastFallTime = GetNowCount();

	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		ClearDrawScreen();

		dropBlock(&block);
		moveBlock(&block);
		// グリッド
		initializeDrawScreen();
		drawBlock(block);

		ScreenFlip();
	}

	DxLib_End();
	return 0;
}

int drawBlock(Block block)
{
	// ===== ミノ描画 =====
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (mino[block.type][y][x] == 1)
			{
				DrawCell(block.x + x, block.y + y, GetBlockColor(block.type));
			}
		}
	}
	return 0;
}
int moveBlock(Block* block)
{
	// ★ エッジ検出用
	static int prevLeft = 0;
	static int prevRight = 0;

	// ===== キー入力 =====
	int nowLeft = CheckHitKey(KEY_INPUT_LEFT);
	int nowRight = CheckHitKey(KEY_INPUT_RIGHT);

	// 左（1回押し）
	if (nowLeft == 1 && prevLeft == 0)
	{
		if (CanMove(*block, -1))
		{
			block->x--;
		}
	}

	// 右（1回押し）
	if (nowRight == 1 && prevRight == 0)
	{
		if (CanMove(*block, 1))
		{
			block->x++;
		}
	}

	prevLeft = nowLeft;
	prevRight = nowRight;
	return 0;
}

int dropBlock(Block* block)
{
	// ★ 落下タイミングの検出
	if (isDropTiming())
	{
		block->y--;//一マス下に落ちる
		if (block->y < 0) block->y = 0; // 地面で止める
	}
	return 0;
}
int isDropTiming(void) {
	static int lastFallTime = 0;
	if (GetNowCount() - lastFallTime >= 1000)
	{
		lastFallTime = GetNowCount();
		return 1; // 落下タイミング
	}
	else
		return 0; // まだ落下タイミングではない
}

void InitBoard(void)
{
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			board[y][x] = EMPTY;
		}
	}
}

// ===== グリッド描画 =====
void initializeDrawScreen(void)
{
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			DrawCell(x, y, GetColor(0, 0, 0));
		}
	}
}

// ===== セル描画 =====
void DrawCell(int x, int y, int color)
{
	Point start = { x, y };
	Point end = { x + 1, y + 1 };

	Point s = GameBoardToScreen(start);
	Point e = GameBoardToScreen(end);

	DrawBox(s.x, s.y, e.x - 1, e.y - 1, color, TRUE);
	DrawBox(s.x, s.y, e.x - 1, e.y - 1, GetColor(255, 255, 255), FALSE);
}

// ===== 色 =====
int GetBlockColor(int type)
{
	switch (type)
	{
	case BLOCK_I: return GetColor(0, 255, 255);
	case BLOCK_O: return GetColor(255, 255, 0);
	case BLOCK_T: return GetColor(128, 0, 128);
	case BLOCK_S: return GetColor(0, 255, 0);
	case BLOCK_Z: return GetColor(255, 0, 0);
	case BLOCK_J: return GetColor(0, 0, 255);
	case BLOCK_L: return GetColor(255, 165, 0);
	}
	return GetColor(255, 255, 255);
}

// ===== 座標変換 =====
Point ConvertTopLeftToBottomLeft(Point p)
{
	Point r;
	r.x = p.x;
	r.y = (WINDOW_HEIGHT - 1) - p.y;
	return r;
}

Point GameBoardToScreen(Point p)
{
	Point r;
	r.x = p.x * CELL_SIZE + MARGIN_X;
	r.y = p.y * CELL_SIZE + MARGIN_Y;
	return ConvertTopLeftToBottomLeft(r);
}

int CanMove(Block block, int dx)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (mino[block.type][y][x] == 1)
			{
				int nx = block.x + x + dx;

				if (nx < 0 || nx >= WIDTH)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}