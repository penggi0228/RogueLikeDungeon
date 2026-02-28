// CmnUserSettingsSaveBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CmnUserSettingsSaveBase.generated.h"

/**
 * 共通ユーザー設定保存ベースSaveGame
 * ・SaveGameの共通メタ情報だけを保持する
 * ・個別カテゴリ(入力/音/画面など)は派生クラスで保持する
 */
UCLASS(Abstract, BlueprintType)
class ROGUELIKEDUNGEON_API UCmnUserSettingsSaveBase : public USaveGame
{
    GENERATED_BODY()

public:
    /** セーブデータバージョン(将来の互換用) */
    UPROPERTY(BlueprintReadWrite, Category = "Common|Save")
    int32 Version = 1;

    /** 保存時刻(デバッグ/表示用。不要なら消してOK) */
    UPROPERTY(BlueprintReadWrite, Category = "Common|Save")
    FDateTime SavedAt;
};