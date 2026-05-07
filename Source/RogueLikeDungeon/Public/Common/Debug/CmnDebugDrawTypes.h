// CmnDebugDrawTypes.h

#pragma once

#include "CoreMinimal.h"
#include "CmnDebugDrawTypes.generated.h"

/**
 * 共通デバッグ描画設定
 */
USTRUCT(BlueprintType)
struct FCmnDebugDrawStyle
{
    GENERATED_BODY()

public:

    // 描画色
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Debug")
    FColor drawColor = FColor::Green;

    // 永続表示するか
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Debug")
    bool bPersistentLines = false;

    // 表示時間
    // bPersistentLines=trueかつ-1.0fの場合は明示的にクリアするまで表示される
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Debug")
    float duration = 10.0f;

    // 線の太さ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Debug", meta = (ClampMin = "0.0"))
    float lineThickness = 1.5f;

    // Z方向オフセット
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Debug")
    float zOffset = 0.0f;

    // セルサイズに対する描画サイズ比率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|Debug", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float sizeScale = 0.25f;

public:

    /**
     * デフォルト描画設定を生成する
     *
     * @param inColor 描画色
     * @param inZOffset Z方向オフセット
     * @param inSizeScale セルサイズに対する描画サイズ比率
     * @param inDuration 表示時間
     * @param inLineThickness 線の太さ
     * @param bInPersistentLines 永続表示するか
     * @return 生成した描画設定
     */
    static FCmnDebugDrawStyle MakeDefault(
        const FColor& inColor,
        float inZOffset,
        float inSizeScale,
        float inDuration = 10.0f,
        float inLineThickness = 1.5f,
        bool bInPersistentLines = false
    )
    {
        FCmnDebugDrawStyle result;
        result.drawColor = inColor;
        result.bPersistentLines = bInPersistentLines;
        result.duration = inDuration;
        result.lineThickness = inLineThickness;
        result.zOffset = inZOffset;
        result.sizeScale = inSizeScale;
        return result;
    }
};
