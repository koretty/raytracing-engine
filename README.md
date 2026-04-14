# RayTracingEngine

CPU ベースの学習向けパストレーサーです。C++20 で、PBR BSDF、BVH、テクスチャ、Beer-Lambert 透過影、HDRI 環境光までを段階的に検証できる構成になっています。

## Overview

`Renderer` は `IBSDF` 抽象に依存し、`Material` はパラメータとテクスチャサンプリングに責務を限定しています。これにより、BSDF の差し替えや拡張を行ってもレンダリング本体への影響を抑えやすい設計です。

## Implemented Features

- OpenMP によるピクセル並列レンダリング（`#pragma omp parallel for schedule(dynamic)`）
- SDL3 ウィンドウ表示と、カメラ移動時の再レンダリング
- `sample / eval / pdf` 契約を持つ PBR BSDF（Lambert + GGX + transmission + ior）
- UV ベースのテクスチャ入力（`SolidColor` / `CheckerTexture` / `ImageTexture`）
- 画像テクスチャ読み込み（P3/P6 PPM）
- Beer-Lambert による厚み依存透過影（ビルドオプションで有効/無効切替）
- BVH（SAH 近似ビニング）による交差判定高速化
- 環境マップによる環境光サンプリング（Radiance `.hdr` の `FORMAT=32-bit_rle_rgbe`）
- ray miss 時に環境マップまたは背景色へフォールバックする経路

## Demo

![Demo](img/test_output1.png)

## Build And Run

### Requirements

- CMake 3.16 以上
- C++20 対応コンパイラ（GCC / Clang / MSVC）
- SDL3（CMake package として検出可能な状態）
- OpenMP（任意。未検出時はシングルスレッド動作）

### Build (Recommended: CMake)

```bash
git clone https://github.com/koretty/raytracing-engine
cd raytracing-engine

cmake -S . -B build
cmake --build build -j
```

主な CMake オプション:

- `-DRAYTRACER_ENABLE_BEER_LAMBERT=OFF`: Beer-Lambert 透過影を無効化
- `-DRAYTRACER_BUILD_TESTS=ON`: `tests/CMakeLists.txt` がある場合にテストターゲットを追加

### Run

Windows:

```powershell
.\build\raytracer.exe
```

Linux/macOS:

```bash
./build/raytracer
```

補足:

- MinGW + SDL3 環境では、`SDL3.dll` が見つかる場合に実行ファイル横へ自動コピーされます。

## Controls

- `W/S/A/D`: 前後左右移動
- `U/Y`: 上下移動
- `P`: 現在フレームを `render_output.ppm` として保存
- `ESC`: 終了

## Configuration

設定はヘッダ内のインライン変数で管理しています。

- カメラ設定: `src/main/config/camera_config.hpp`
- シーン設定: `src/main/config/scene_config.hpp`

既定値の要点:

- `config::scene::benchmark::enabled = true`
  - 既定で `11 x 11` の球体グリッド（`grid_half_extent = 5`）を生成します。
- `config::environment::enabled = false`
  - 既定では HDRI 無効で、背景色を使用します。
- 画像テクスチャは既定で `img/red_matte.ppm` を読み込み、失敗時は `SolidColor` にフォールバックします。

HDRI を有効化する場合は `scene_config.hpp` の `config::environment` を変更します。

```cpp
namespace config {
namespace environment {

inline bool enabled = true;
inline const char* hdr_path = "img/environment.hdr";
inline float intensity = 1.0f;

} // namespace environment
} // namespace config
```

補足:

- HDRI の読み込みに失敗した場合はログにエラーを出力し、背景色フォールバックで継続します。
- 対応 HDR 形式は Radiance の RLE RGBE (`FORMAT=32-bit_rle_rgbe`) です。

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
