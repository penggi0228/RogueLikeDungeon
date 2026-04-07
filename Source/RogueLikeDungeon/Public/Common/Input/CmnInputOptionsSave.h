// CmnInputOptionsSave.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Save/CmnUserSettingsSaveBase.h"
#include "CmnInputOptionsSave.generated.h"

/**
 * 共通入力オプション保存用SaveGame
 * 入力オプション設定値を保存する
 */
UCLASS(BlueprintType)
class ROGUELIKEDUNGEON_API UCmnInputOptionsSave : public UCmnUserSettingsSaveBase
{
    GENERATED_BODY()

public:

    // カメラ左右反転設定
    UPROPERTY(BlueprintReadWrite, Category = "Common|Input|Camera")
    bool bInvertCameraX = false;

    // カメラ上下反転設定
    UPROPERTY(BlueprintReadWrite, Category = "Common|Input|Camera")
    bool bInvertCameraY = false;

    // カメラ感度
    // 将来の設定項目追加に備えて保持
    UPROPERTY(BlueprintReadWrite, Category = "Common|Input|Camera")
    float CameraSensitivity = 1.0f;
};
