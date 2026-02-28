// CmnInputOptionsSave.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Save/CmnUserSettingsSaveBase.h"
#include "CmnInputOptionsSave.generated.h"

/**
 * 共通入力オプション保存用SaveGame
 * ・反転/感度など「挙動系」の設定を保存する
 * ・キー割り当てはEnhancedInputUserSettings側に保存する方針
 */
UCLASS(BlueprintType)
class ROGUELIKEDUNGEON_API UCmnInputOptionsSave : public UCmnUserSettingsSaveBase
{
    GENERATED_BODY()

public:
    /** カメラ左右反転 */
    UPROPERTY(BlueprintReadWrite, Category = "Common|Input|Camera")
    bool bInvertCameraX = false;

    /** カメラ上下反転 */
    UPROPERTY(BlueprintReadWrite, Category = "Common|Input|Camera")
    bool bInvertCameraY = false;

    /** カメラ感度(将来用) */
    UPROPERTY(BlueprintReadWrite, Category = "Common|Input|Camera")
    float CameraSensitivity = 1.0f;
};