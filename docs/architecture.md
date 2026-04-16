# Architecture Overview

このプロジェクトは、CPU パストレーサーの学習実装をモジュール分割した構成です。現行コードでは、BSDF 抽象化、BVH 加速、UV テクスチャ、Beer-Lambert 透過影、HDRI 環境サンプリングまでが 1 本の経路で統合されています。

## 設計原則

- 依存逆転: `Renderer` は `IBSDF` 抽象に依存し、既定実装は `PbrBsdf`。
- 責務分離: `Material` はデータ保持とテクスチャ取得に限定し、散乱評価は `bsdf` に集約。
- フォールバック重視: 環境マップ未設定時は背景色へ、画像テクスチャ読込失敗時は `SolidColor` へフォールバック。
- 加速構造の遅延更新: `Scene` は `bvh_dirty` を使って BVH を遅延再構築。
- 設定集中: シーン・カメラ既定値は `src/main/config` に集約。

## モジュール責務

- `src/main/main.cpp`
    - SDL イベントループ、カメラ移動入力、再レンダリング制御、PPM 保存。
- `src/main/config/camera_config.hpp`
    - `origin/lookat/vup/fov/aperture/move_speed` を定義し `make_camera` でカメラ生成。
- `src/main/config/scene_config.hpp`
    - マテリアル、球体配置、太陽光、HDRI 設定を使って `Scene` を構築。
- `src/renderer/renderer.{hpp,cpp}`
    - OpenMP 画素ループ、`trace_ray` 再帰、直達太陽光、透過影、環境サンプリング。
- `src/bsdf/bsdf.hpp`
    - `IBSDF` 契約（`sample/eval/pdf`）と `BsdfSample`。
- `src/bsdf/pbr_bsdf.{hpp,cpp}`
    - Lambert + GGX + transmission/ior を混合したサンプリングと評価。
- `src/material/material.hpp`
    - `albedo_texture` / `emission_texture` と物性パラメータを保持。
- `src/material/texture.{hpp,cpp}`
    - `SolidColor` / `CheckerTexture` / `ImageTexture`（PPM P3/P6）を提供。
- `src/material/optics.hpp`
    - `beer_lambert_transmittance` による吸収計算。
- `src/environment/environment_map.{hpp,cpp}`
    - Radiance HDR（RGBE RLE）読込、lat-long 方向サンプリング、双線形補間。
- `src/scene/scene.{hpp,cpp}`
    - オブジェクト/マテリアル所有、環境設定、BVH 構築管理、交差検索 API。
- `src/object/bvh.{hpp,cpp}`
    - SAH 近似ビニングで BVH 構築、軸に応じたトラバーサル順最適化。
- `src/object/sphere.cpp`
    - 交差判定時に球面 UV を計算し `HitRecord` へ格納。

## 実行時フロー

1. `main` が `config::scene::create_scene()` で `Scene` を生成。
2. `main` が `needs_render` のときだけ `Renderer::render(scene, camera)` を呼ぶ。
3. `render` 開始時に `scene.prepare_acceleration()` を実行して BVH を事前準備。
4. 各ピクセル・各サンプルで `camera.get_ray(u, v)` を発行し `trace_ray` を再帰実行。
5. hit 時:
     - `HitRecord` から `material_id/u/v/object_id/front_face` を利用。
     - `mat.sample_emission` で自己発光。
     - `bsdf.eval` + `evaluate_shadow_transmittance` で太陽の直達光。
     - `bsdf.sample` で次方向を生成し、`sampled.weight * trace_ray(...)` を加算。
     - 屈折体内部区間は Beer-Lambert で `segment_transmittance` を適用。
6. miss 時:
     - `scene.sample_environment(ray.getDirection())` が HDRI または背景色を返す。

## BVH とスレッド安全性

- `Scene::add_object` は `object_id` を付与し、`bvh_dirty = true` に設定。
- `Scene::prepare_acceleration` / `find_closest_hit` は mutex と atomic で二重構築を抑止。
- `BVHNode` は `kLeafSize = 4`, `kBinCount = 16` の SAH 近似ビニング。

## HDRI 実装範囲

- 入力形式: Radiance `.hdr`（`FORMAT=32-bit_rle_rgbe`）
- 読み込み: scanline RLE デコード + RGBE から float RGB へ復元
- サンプリング: 方向ベクトル -> 緯度経度 UV 変換 -> 双線形補間
- 強度: `EnvironmentMap::set_intensity(float)`

## ビルド構成

- `raytracer_core`: `src/main/main.cpp` 以外のコア実装をまとめた静的ライブラリ
- `raytracer`: SDL エントリーポイントを持つ実行バイナリ
- OpenMP は検出時にリンク。未検出時も実行可能（並列化なし）
