# RayTracingEngine

CPU ベースの学習向けパストレーサーです。

## Overview

`Renderer` は `IBSDF` 抽象に依存し、`Material` は「パラメータ保持 + テクスチャサンプリング」に責務を限定しています。散乱評価ロジックは `PbrBsdf` に分離されているため、BSDF 実装の差し替えや拡張をレンダラー本体へ波及させにくい構造です。

## Implemented Features

- OpenMP によるピクセル並列レンダリング（`#pragma omp parallel for schedule(dynamic)`）
- SDL3 ウィンドウ表示、カメラ移動時の再レンダリング
- `sample / eval / pdf` 契約を持つ PBR BSDF（Lambert + GGX + transmission + ior）
- UV ベースのテクスチャ入力（`SolidColor` / `CheckerTexture` / `ImageTexture`）
- 画像テクスチャ読み込み（P3/P6 PPM）
- Beer-Lambert による厚み依存透過影（ビルドオプションで有効/無効切替）
- BVH（SAH 近似ビニング）による交差判定高速化
- 環境マップによる環境光サンプリング（Radiance `.hdr` の `FORMAT=32-bit_rle_rgbe`）
- ray miss 時に環境マップまたは背景色へフォールバックする経路
- 球体交差時の UV 生成と `HitRecord` への `u/v/object_id` 反映

## Demo

![Demo](img/test_output1.png)

## Build And Run

### Requirements

- C++20 対応コンパイラ（GCC / Clang / MSVC）
- SDL3 開発パッケージ（ヘッダ + ライブラリ）
- OpenMP（任意。未検出時はシングルスレッド動作）

### Build (Direct Command, No CMake)


```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic \
	-I./src \
	./src/main/main.cpp \
	./src/bsdf/pbr_bsdf.cpp \
	./src/environment/environment_map.cpp \
	./src/material/texture.cpp \
	./src/object/bvh.cpp \
	./src/object/sphere.cpp \
	./src/renderer/renderer.cpp \
	./src/scene/scene.cpp \
	-o ./raytracer \
	$(pkg-config --cflags --libs sdl3) \
	-fopenmp
```

## Controls

- `W/S/A/D`: 前後左右移動
- `U/Y`: 上下移動
- `P`: 現在フレームを `render_output.ppm` として保存
- `ESC`: 終了

## Runtime Defaults

`src/main/main.cpp` の既定値:

- 画面サイズ: `800 x 600`
- サンプル数: `100 spp`
- 最大再帰深度: `10`


## Documentation

- [Architecture](docs/architecture.md)
- [Class Diagram](docs/class_diagram.md)
- [Data Flow](docs/data_flow.md)
- [Dependency Graph](docs/dependency.md)
- [Recommendations](docs/recommendations.md)

## Project Structure

```text
.
├── docs/
├── img/
├── src/
│   ├── bsdf/                # IBSDF 抽象と PbrBsdf 実装
│   ├── environment/         # HDRI ローダと方向サンプリング
│   ├── main/
│   │   ├── config/          # camera / scene 設定
│   │   └── main.cpp
│   ├── material/            # Material データ + Texture + 光学補助
│   ├── math/                # Vec3 / Ray / random
│   ├── object/              # Object 抽象 + Sphere + BVH + AABB
│   ├── renderer/            # パストレース本体
│   └── scene/               # Scene / Camera
├── CMakeLists.txt
└── README.md
```

## Roadmap

- [x] PBR BSDF の `sample/eval/pdf` 分離
- [x] Beer-Lambert 透過影
- [x] HDRI 環境マップ対応
- [x] BVH 導入による交差判定高速化
- [ ] 環境光の importance sampling と MIS
- [ ] Russian roulette による経路終了
- [ ] 自動テスト整備（BSDF/交差/BVH/HDR ローダ）

## License

MIT License
