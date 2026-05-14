# teto 開発者ドキュメント

> このファイルは Claude Code (claude.ai/code) の参照ガイドも兼ねています。

---

## 目次

1. [プロジェクト概要](#プロジェクト概要)
2. [ビルド手順](#ビルド手順)
3. [DxLib セットアップ](#dxlib-セットアップ)
4. [ファイル構成](#ファイル構成)
5. [アーキテクチャ](#アーキテクチャ)
6. [関数リファレンス](#関数リファレンス)
7. [定数・グローバル変数](#定数グローバル変数)
8. [既知の制限事項](#既知の制限事項)

---

## プロジェクト概要

C++ / DxLib で実装したシングルファイルのテトリスクローン。
`main.cpp` 1ファイルにすべてのロジック・描画・入力処理が収まっている。

- 言語: C++ (C スタイル中心)
- グラフィックライブラリ: DxLib
- ターゲット: Windows x64

---

## ビルド手順

### Visual Studio 2022

`tetris.sln` をダブルクリックして開き、`F5` (Debug) または `Ctrl+F5` (Release) でビルド・実行。

### Developer Command Prompt

```bat
msbuild tetris.sln /p:Configuration=Debug /p:Platform=x64
msbuild tetris.sln /p:Configuration=Release /p:Platform=x64
```

成果物は `x64\Debug\tetris.exe` または `x64\Release\tetris.exe` に出力される。

### テスト

テストフレームワークは未導入。動作確認はビルドして実行するだけ。

---

## DxLib セットアップ

`tetris.vcxproj` の `DX_LIB_DIR` プロパティが DxLib のインストールパスを指している。

```
C:\DxLib_VC\プロジェクトに追加すべきファイル_VC用
```

| 構成 | リンクライブラリ |
|------|----------------|
| x64 Release / Debug | `$(DX_LIB_DIR)\lib_x64\DxLib_x64.lib` |
| Win32 Release / Debug | `$(DX_LIB_DIR)\lib\DxLib.lib` |

別の環境にクローンした場合は `DX_LIB_DIR` を実際のパスに変更すること。

---

## ファイル構成

```
teto/
├── main.cpp              # ゲーム全コード
├── tetris.sln            # Visual Studio ソリューション
├── tetris.vcxproj        # プロジェクト設定
├── manual.md             # 開発者ドキュメント（本ファイル）
├── playguide.md          # ユーザー向けプレイガイド
├── x64/
│   └── Debug/
│       └── tetris.exe    # ビルド成果物
└── tetris/
    └── x64/Debug/        # 中間ファイル (.obj, .log など)
```

---

## アーキテクチャ

### ウィンドウレイアウト

```
WINDOW_WIDTH = CELL_SIZE * WIDTH + MARGIN_X * 2 + SCORE_PANEL_WIDTH
             = 30 * 10 + 60 * 2 + 130
             = 550 px

WINDOW_HEIGHT = CELL_SIZE * HEIGHT + MARGIN_Y * 2
              = 30 * 20 + 10 * 2
              = 620 px

┌── 60px ──┬──── 300px ────┬──── 130px ────┐
│  左余白   │  ゲームフィールド │   スコアパネル  │  620px
└──────────┴───────────────┴───────────────┘
```

スコアパネルの描画開始 X 座標: `MARGIN_X + CELL_SIZE * WIDTH + 15 = 375 px`

### 座標系

盤面とスクリーンで原点と Y 軸方向が異なる。

| 座標系 | 原点 | Y 軸 |
|--------|------|------|
| 盤面座標 | 左下 | 上方向が増加（y=0 が最下段） |
| スクリーン座標 (DxLib) | 左上 | 下方向が増加 |

変換は `GameBoardToScreen()` → `ConvertTopLeftToBottomLeft()` の 2 段構成。

```
盤面 (x, y)
  → GameBoardToScreen: pixel_x = x * CELL_SIZE + MARGIN_X
                       pixel_y = y * CELL_SIZE + MARGIN_Y
  → ConvertTopLeftToBottomLeft: screen_y = (WINDOW_HEIGHT - 1) - pixel_y
```

### 盤面データ `board[HEIGHT][WIDTH]`

```cpp
int board[20][10];
```

- `0 (EMPTY)` = 空セル
- `1〜7` = 固定済みブロックの種別 (`block.type + 1` で格納)
- 参照時は `-1` して `GetBlockColor()` に渡す

### ブロック種別と色

| enum 値 | 名前 | 色 |
|--------|------|----|
| 0 BLOCK_I | アイ | シアン (0, 255, 255) |
| 1 BLOCK_O | オー | イエロー (255, 255, 0) |
| 2 BLOCK_T | ティー | パープル (128, 0, 128) |
| 3 BLOCK_S | エス | グリーン (0, 255, 0) |
| 4 BLOCK_Z | ゼット | レッド (255, 0, 0) |
| 5 BLOCK_J | ジェイ | ブルー (0, 0, 255) |
| 6 BLOCK_L | エル | オレンジ (255, 165, 0) |

### ミノ形状と回転 `mino[7][4][4]`

`mino[type][y][x]` に rotation=0 のときの初期形状を定義する。
`GetMinoCell(type, rot, x, y)` が回転変換を行う。

| rot | 変換式 | 意味 |
|-----|--------|------|
| 0 | `mino[type][y][x]` | 初期形状 |
| 1 | `mino[type][x][3-y]` | 時計回り 90° |
| 2 | `mino[type][3-y][3-x]` | 180° |
| 3 | `mino[type][3-x][y]` | 反時計回り 90° |

### スコアシステム

グローバル変数 `score` / `totalLines` で管理する。

```cpp
int score = 0;       // 累計スコア
int totalLines = 0;  // 累計消去ライン数
```

`eraseBlock()` がライン消去時に以下のテーブルでスコアを加算する。

| 同時消去ライン数 | 加算スコア |
|:--------------:|----------:|
| 1 | 100 |
| 2 | 300 |
| 3 | 500 |
| 4 | 800 |

```cpp
static const int scoreTable[] = {0, 100, 300, 500, 800};
score += scoreTable[lines];  // lines: 1〜4
totalLines += lines;
```

### メインループのフロー

```
while (ProcessMessage() == 0 && ESC押されていない)
│
├─ ClearDrawScreen()
├─ initializeDrawScreen()   黒セルで盤面塗りつぶし
│
├─ dropBlock()              落下タイミングなら 1 段下げ
│   └─ 着地したら board に書き込み → stucked=1 を返す
│
├─ [stucked=1 のとき]
│   ├─ placeNew()           新ブロックを生成（スポーン: x=3, y=18）
│   └─ CheckGameOver()      ゲームオーバー判定 → 真なら暗転アニメ後 exit
│
├─ moveBlock()              左右・ハードドロップ キー処理
├─ rotateBlock()            X/Z キー処理
├─ eraseBlock()             満ライン消去 + スコア加算
├─ drawBlock()              操作中ブロックの描画
├─ drawBoard()              固定済みブロック（board 配列）の描画
├─ drawScore()              右パネルにスコア・ライン数表示
└─ ScreenFlip()             バックバッファを表示
```

### キー入力のエッジ検出

`moveBlock()` と `rotateBlock()` はそれぞれ `static int prev*` 変数で前フレームのキー状態を保持し、`now==1 && prev==0` のときのみ処理する。これにより「1回押し = 1アクション」を実現している（オートリピートなし）。

```cpp
static int prevLeft = 0;
if (nowLeft == 1 && prevLeft == 0) { /* 処理 */ }
prevLeft = nowLeft;
```

### 落下タイミング `isDropTiming()`

```cpp
static int lastFallTime = 0;
if (GetNowCount() - lastFallTime >= dropInterval) {
    lastFallTime = GetNowCount();
    return 1;
}
```

`static` 変数で最終落下時刻を保持し、経過時間が `dropInterval`（現在 100ms）を超えたときに 1 を返す。

### ゲームオーバー判定 `CheckGameOver()`

`placeNew()` で新ブロックを配置した直後に呼び出す。新ブロックのセルと `board` の既存データが重なっていれば 1 を返す。

> **注意**: `placeNew()` は `block.rotation` をリセットしないため、前のブロックの回転状態が引き継がれる。現状は `block.type` の変更が `mino` データを切り替えるので実害はないが、意図的な設計ではない。

---

## 関数リファレンス

| 関数 | 説明 |
|------|------|
| `WinMain()` | エントリポイント。初期化・メインループ・終了処理 |
| `InitBoard()` | `board` 配列を EMPTY で初期化 |
| `initializeDrawScreen()` | 全セルを黒で描画（フレームごとの背景クリア） |
| `DrawCell(x, y, color)` | 盤面座標 (x,y) に塗りつぶし＋白枠のセルを描画 |
| `drawBlock(block)` | 操作中ブロックを描画 |
| `drawBoard(block)` | `board` 配列の固定済みブロックを描画 |
| `drawScore()` | 右パネルに SCORE・LINES を描画 |
| `GetBlockColor(type)` | ブロック種別から DxLib カラーコードを返す |
| `moveBlock(block*)` | 左右・ハードドロップ キー入力を処理 |
| `rotateBlock(block*)` | X/Z キー入力で回転処理 |
| `dropBlock(block*, interval)` | タイマー落下 + 着地時 board 書き込み。着地したら 1 を返す |
| `eraseBlock()` | 満ライン消去・上段を詰める・スコア加算。消去ライン数を返す |
| `placeNew(block*)` | 新ブロックをスポーン位置 (x=3, y=18) に配置 |
| `isDropTiming(interval)` | 落下タイミングを判定。タイミングなら 1 を返す |
| `CanMove(block, dx, dy)` | 移動後に壁・既存ブロックと衝突するか判定 |
| `CanRotate(block, dir)` | 回転後に壁・既存ブロックと衝突するか判定 |
| `CheckGameOver(block)` | 新ブロックが既存ブロックと重なっていれば 1 を返す |
| `GetMinoCell(type, rot, x, y)` | 回転変換済みのセル値を返す |
| `GameBoardToScreen(p)` | 盤面座標 → スクリーン座標に変換 |
| `ConvertTopLeftToBottomLeft(p)` | Y 軸を反転（左上原点 → 左下原点） |

---

## 定数・グローバル変数

### マクロ定数

| 定数 | 値 | 説明 |
|------|----|------|
| `WIDTH` | 10 | 盤面の横マス数 |
| `HEIGHT` | 20 | 盤面の縦マス数 |
| `CELL_SIZE` | 30 | 1セルのピクセルサイズ |
| `MARGIN_X` | 60 | 盤面の左右マージン |
| `MARGIN_Y` | 10 | 盤面の上下マージン |
| `SCORE_PANEL_WIDTH` | 130 | 右スコアパネルの幅 |
| `WINDOW_WIDTH` | 550 | ウィンドウ幅 (計算値) |
| `WINDOW_HEIGHT` | 620 | ウィンドウ高さ (計算値) |
| `EMPTY` | 0 | 空セルを表す値 |

### グローバル変数

| 変数 | 型 | 説明 |
|------|----|------|
| `board[HEIGHT][WIDTH]` | `int` | 固定済みブロックの盤面データ |
| `score` | `int` | 現在のスコア |
| `totalLines` | `int` | 累計消去ライン数 |
| `scoreFont` | `int` | スコア表示用フォントハンドル |
| `mino[7][4][4]` | `int` | 全ミノの初期形状データ |

---

## 既知の制限事項

- **スポーン回転引継ぎ**: `placeNew()` が `rotation` をリセットしないため、前ブロックの回転状態が新ブロックに引き継がれる
- **難易度固定**: 落下速度 (`interval = 100ms`) がゲーム全体を通して変化しない
- **ネクストブロック未表示**: 次に来るブロックのプレビューがない
- **ソフトドロップなし**: 下キーはハードドロップ（即底落とし）のみ。1マスずつ加速するソフトドロップは未実装
- **ハイスコア非永続化**: スコアはメモリ上のみで管理され、ゲーム終了時に消える
- **SRS (Super Rotation System) 未対応**: 壁蹴り (wall kick) なし。回転スペースがない場合は単純に回転拒否
