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

int mino[7][4][4] =
{
	{
		{0,0,0,0},
		{1,1,1,1},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{0,1,1,0},
		{0,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{0,1,0,0},
		{1,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{0,1,1,0},
		{1,1,0,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{1,1,0,0},
		{0,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{1,0,0,0},
		{1,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	},
	{
		{0,0,1,0},
		{1,1,1,0},
		{0,0,0,0},
		{0,0,0,0}
	}
};

Point ConvertTopLeftToBottomLeft(Point p);
Point GameBoardToScreen(Point p);
void initializeDrawScreen(void);
void DrawCell(int x, int y, int color);
void InitBoard(void);
int GetBlockColor(int type);
int CanMove(Block block, int dx, int dy);
int isDropTiming(int);
int dropBlock(Block* block, int dropInterval);
int moveBlock(Block* block);
int drawBlock(Block block);
int drawBoard(Block block);
int GetMinoCell(int type, int rot, int x, int y);
int CanRotate(Block block, int dir);
int rotateBlock(Block* block);
int eraseBlock(void);
void placeNew(Block* block);
int CheckGameOver(Block block);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	int interval = 100; // ブロックの落下間隔（ミリ秒）
	srand((unsigned int)time(NULL));
	ChangeWindowMode(TRUE);
	SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);
	if (DxLib_Init() == -1) return -1;
	SetDrawScreen(DX_SCREEN_BACK);

	int gameOverFont = CreateFontToHandle(
		_T("Arial"),
		40,
		3
	);

    InitBoard();

    // テストブロック
    Block block;
    block.x = 3;
    block.y = 18;
    block.type = BLOCK_T;
    block.rotation = 0;

    int lastFallTime = GetNowCount();

	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		ClearDrawScreen();
		initializeDrawScreen();
		if (dropBlock(&block, interval))
		{
			placeNew(&block);
			if (CheckGameOver(block))
			{
				int alpha = 0;

				// 徐々に暗転
				while (alpha <= 180)
				{
					ClearDrawScreen();

					initializeDrawScreen();
					drawBoard(block);

					// 今のミノも描画
					drawBlock(block);

					// 半透明黒
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

					DrawBox(
						0,
						0,
						WINDOW_WIDTH,
						WINDOW_HEIGHT,
						GetColor(0, 0, 0),
						TRUE
					);

					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

					// 少し暗くなってから文字表示
					if (alpha >= 100)
					{
						DrawStringToHandle(
							105,
							270,
							_T("GAME OVER"),
							GetColor(255, 0, 0),
							gameOverFont
						);
					}

					ScreenFlip();

					alpha += 5;

					WaitTimer(30);
				}

				// 最後に少し止める
				WaitTimer(1500);

				DxLib_End();
				exit(0);
			}
		}
		moveBlock(&block);
		rotateBlock(&block);
		eraseBlock();
		drawBlock(block);
		drawBoard(block);
		DxLib::ScreenFlip();
	}

	DxLib::DxLib_End();
	return 0;
}

int drawBoard(Block block)
{
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			if (board[y][x] != EMPTY)
			{
				DrawCell(x, y, GetBlockColor(board[y][x] - 1));
			}
		}
	}
	return 0;
}

int drawBlock(Block block)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (GetMinoCell(block.type, block.rotation, x, y))
			{
				DrawCell(block.x + x, block.y + y, GetBlockColor(block.type));
			}
		}
	}
	return 0;
}

int moveBlock(Block* block)
{
	static int prevLeft = 0;
	static int prevRight = 0;
	static int prevDown = 0;

	int nowLeft = CheckHitKey(KEY_INPUT_LEFT);
	int nowRight = CheckHitKey(KEY_INPUT_RIGHT);
	int nowDown = CheckHitKey(KEY_INPUT_DOWN);

	if (nowLeft == 1 && prevLeft == 0)
	{
		if (CanMove(*block, -1, 0))
		{
			block->x--;
		}
	}

	if (nowRight == 1 && prevRight == 0)
	{
		if (CanMove(*block, 1, 0))
		{
			block->x++;
		}
	}
	if (nowDown == 1 && prevDown == 0)
	{
		while (CanMove(*block, 0, -1))
		{
			block->y--;
		}
	}

	prevLeft = nowLeft;
	prevRight = nowRight;
	prevDown = nowDown;
	return 0;
}

int eraseBlock()
{
	int x, y;
	for(y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			if (board[y][x] == EMPTY)
				break;
		}
		if (x == WIDTH)
		{
			for (int yy = y; yy < HEIGHT - 1; yy++)
			{
				for (int xx = 0; xx < WIDTH; xx++)
				{
					board[yy][xx] = board[yy + 1][xx];
				}
			}
			for (int xx = 0; xx < WIDTH; xx++)
			{
				board[HEIGHT - 1][xx] = EMPTY;
			}
			y--;
		}
	}
	return 0;
}

int dropBlock(Block* block, int dropInterval)
{
	int stucked = 0;
	// ★ 落下タイミングの検出
	if (isDropTiming(dropInterval))
	{
		if (CanMove(*block, 0, -1))
		{
			block->y--;
		}
		else
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					if (GetMinoCell(block->type, block->rotation, x, y))
					{
						int bx = block->x + x;
						int by = block->y + y;

						if (by >= 0 && by < HEIGHT && bx >= 0 && bx < WIDTH)
						{
							board[by][bx] = block->type + 1;
						}
					}
				}
			}

			stucked = 1;

		}
	}
	return stucked;
}
void placeNew(Block* block)
{
	block->x = 3;
	block->y = 18;
	block->type = rand() % 7;
}

int isDropTiming(int dropInterval) {
	static int lastFallTime = 0;
	if (GetNowCount() - lastFallTime >= dropInterval)
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

void DrawCell(int x, int y, int color)
{
	Point start = { x, y };
	Point end = { x + 1, y + 1 };

	Point s = GameBoardToScreen(start);
	Point e = GameBoardToScreen(end);

	DrawBox(s.x, s.y, e.x - 1, e.y - 1, color, TRUE);
	DrawBox(s.x, s.y, e.x - 1, e.y - 1, GetColor(255, 255, 255), FALSE);
}

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

int CanMove(Block block, int dx, int dy)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (GetMinoCell(block.type, block.rotation, x, y))
			{
				int nx = block.x + x + dx;
				int ny = block.y + y + dy;

				if (nx < 0 || nx >= WIDTH || ny < 0)
					return 0;

				if (ny < HEIGHT && board[ny][nx] != EMPTY)
					return 0;
			}
		}
	}
	return 1;
}

//回転後の値を取得
int GetMinoCell(int type, int rot, int x, int y)
{
	switch (rot % 4)
	{
		case 0: return mino[type][y][x];
		case 1: return mino[type][x][3 - y];
		case 2: return mino[type][3 - y][3 - x];
		case 3: return mino[type][3 - x][y];
	}
	return 0;
}

//回転判定
int CanRotate(Block block, int dir)
{
	int nextRot = (block.rotation + dir + 4) % 4;

	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (GetMinoCell(block.type, nextRot, x, y))
			{
				int nx = block.x + x;
				int ny = block.y + y;

				if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT)
				{
					return 0;
				}
				if (board[ny][nx] != EMPTY)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

//回転処理
int rotateBlock(Block* block)
{
	static int prevX = 0;
	static int prevZ = 0;

	int nowX = CheckHitKey(KEY_INPUT_X);
	int nowZ = CheckHitKey(KEY_INPUT_Z);

	// 時計回り（Xキー）
	if (nowX == 1 && prevX == 0)
	{
		if (CanRotate(*block, 1))
		{
			block->rotation = (block->rotation + 1) % 4;
		}
	}

	// 反時計回り（Zキー）
	if (nowZ == 1 && prevZ == 0)
	{
		if (CanRotate(*block, -1))
		{
			block->rotation = (block->rotation - 1 + 4) % 4;
		}
	}

	prevX = nowX;
	prevZ = nowZ;
	return 0;
}
int CheckGameOver(Block block)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (mino[block.type][y][x] == 0)
            {
                continue;
            }

            int bx = block.x + x;
            int by = block.y + y;

            if (by < HEIGHT && board[by][bx] != EMPTY)
            {
                return 1;
            }
        }
    }

    return 0;
}
