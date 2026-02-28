// CmnSaveGameLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CmnSaveGameLibrary.generated.h"

class UCmnUserSettingsSaveBase;

/**
 * 共通SaveGameユーティリティ
 * ・ロード/作成/保存の共通処理をまとめる
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnSaveGameLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * スロットからロード。無ければ新規作成して返す。
     * @param SaveClass 作成するSaveGameクラス
     * @param SlotName スロット名
     * @param UserIndex ユーザーIndex
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Save")
    static UCmnUserSettingsSaveBase* LoadOrCreate(TSubclassOf<UCmnUserSettingsSaveBase> SaveClass, const FString& SlotName, int32 UserIndex);

    /**
     * SaveGameを保存する
     * @param SaveObject 保存対象
     * @param SlotName スロット名
     * @param UserIndex ユーザーIndex
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Save")
    static bool Save(UCmnUserSettingsSaveBase* SaveObject, const FString& SlotName, int32 UserIndex);
};