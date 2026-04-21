# システムアーキテクチャ 

このドキュメントでは、本プロジェクトの全体構造、使用技術、およびディレクトリ構成について記述します。

## 1. システム全体図
システムの全体像と、各コンポーネント間の相互作用です。

```mermaid
graph TD
    %% 外部のヘッダーファイルやモジュール
    Scene["scene_config.hpp"]
    Camera["camera_config.hpp"]
    Renderer["renderer.hpp"]
    SDL3["SDL3(外部ライブラリ)"]

    subgraph MainContainer [" "]
        direction TB

        Title{"main.cpp"}
        style Title fill:none,stroke:none,font-size:18px,font-weight:bold
        
        Step1["1. 情報の読み込みと初期化"]
        Step2["2. 描画処理"]
        Step3["3. 描画出力"]
    
        Title ~~~ Step1
        Step1 --> Step2 --> Step3
    end

    Scene -->|シーン情報| Step1
    Camera -->|カメラ情報| Step1
    Renderer -->|描画関数| Step2
    SDL3-->Step3 

```

## 2. 技術スタック 


## 3. ディレクトリ構成
プロジェクトの主要なディレクトリ構造とその役割です。
```text

src/
├── main/
│   ├── main.cpp                # エントリーポイント
│   └── config/
│       ├── scene_config.hpp    # シーン設定
│       └── camera_config.hpp   # カメラ設定
```

