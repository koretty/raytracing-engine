# 改善提案（現状コード基準）

## 実装済み

- `IBSDF` 導入による散乱責務分離（`sample/eval/pdf`）
- `PbrBsdf` による Lambert + GGX + transmission/ior の統合
- 透明影の Beer-Lambert 減衰
- BVH（SAH 近似ビニング）による交差探索加速
- HDRI 環境マップ（Radiance RGBE RLE）ロードと双線形サンプリング
- テクスチャ入力マテリアル（`SolidColor` / `CheckerTexture` / `ImageTexture`）
- 球面 UV 生成と `HitRecord` への `u/v/object_id` 格納
- CMake で `raytracer_core` と `raytracer` の分離

## 優先度 High

1. 環境光の importance sampling + MIS
- 現状は miss 時サンプリング中心で、高輝度 HDRI 下では収束速度が不足しやすいです。

2. Russian roulette 終了
- `max_depth` 依存の固定終了を補完し、平均計算量を抑えながらバイアスなしで高速化できます。

3. 自動テスト整備
- 最小セットとして以下を推奨します。
- `Sphere::hit` の境界ケース
- BVH あり/なしでの hit 一致
- `PbrBsdf::pdf` の非負性と整合
- HDR ローダ（正常系/異常系）

4. 直接光サンプリング拡張
- 現状の太陽光は固定方向ライトのみです。面光源や発光オブジェクトに拡張するとノイズ特性を改善できます。

## 優先度 Medium

1. トーンマッピング
- 現状はガンマ補正（`sqrt`）中心のため、HDR 出力の見栄えと露出制御を改善できます。

2. テクスチャ品質改善
- `ImageTexture` は現在 nearest サンプリングです。mipmap やフィルタリング導入で遠景品質を改善できます。

3. 設定外部化
- `src/main/config/*.hpp` のインライン設定を JSON/TOML 化すると、再ビルドなしで調整できます。

4. 性能計測の定型化
- benchmark グリッド（既定 11x11）を活用し、解像度・spp・スレッド数別の計測スクリプトを追加すると改善効果を継続比較できます。

## 設計上の維持ポイント

- `Renderer` の散乱評価は `IBSDF` 抽象依存を維持し、具象 BSDF の注入可能性を保つ。
- `Scene::sample_environment` を環境光取得の唯一窓口として維持する。
- `Material` はデータとテクスチャ取得に限定し、評価ロジックは `bsdf` / `material_optics` に閉じ込める。