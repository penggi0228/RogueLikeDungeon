// CmnGridDefinition.h

#pragma once

#include "CoreMinimal.h"
#include "CmnGridDefinition.generated.h"

/**
 * グリッド定義構造体
 * グリッド座標とワールド座標の変換に使用する値を保持する
 */
USTRUCT(BlueprintType)
struct ROGUELIKEDUNGEON_API FCmnGridDefinition
{
    GENERATED_BODY()

public:

    // ----- コンストラクタ -----

    FCmnGridDefinition() = default;

public:

    // ----- グリッド定義 -----

    // 1マスのワールドサイズ
    // 0以下だと変換不能になるため、通常は1以上を設定する
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Grid", meta = (ClampMin = "1.0"))
    float cellSize = 100.0f;

     // グリッド原点ワールド座標
     // グリッド座標(0,0)が対応するマス中心のワールド座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Grid")
    FVector originWorld = FVector::ZeroVector;
};
