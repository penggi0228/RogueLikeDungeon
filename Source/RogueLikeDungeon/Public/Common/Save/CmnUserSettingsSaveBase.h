// CmnUserSettingsSaveBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CmnUserSettingsSaveBase.generated.h"

/**
 * 共通ユーザー設定保存用ベースSaveGame
 * 共通メタ情報を保持する
 */
UCLASS(Abstract, BlueprintType)
class ROGUELIKEDUNGEON_API UCmnUserSettingsSaveBase : public USaveGame
{
    GENERATED_BODY()

public:

    // セーブデータバージョン
    UPROPERTY(BlueprintReadWrite, Category = "Common|Save")
    int32 Version = 1;

    // 保存時刻
    // デバッグ表示や保存日時表示に使用
    UPROPERTY(BlueprintReadWrite, Category = "Common|Save")
    FDateTime SavedAt;
};
