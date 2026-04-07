// CmnSaveGameLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CmnSaveGameLibrary.generated.h"

class UCmnUserSettingsSaveBase;

/**
 * 共通SaveGameユーティリティ
 * SaveGameのロードと保存を行う
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnSaveGameLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * SaveGameをロードまたは新規作成する
     *
     * @param SaveClass SaveGameクラス
     * @param SlotName スロット名
     * @param UserIndex ユーザーIndex
     * @return ロードまたは新規作成したSaveGame
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Save")
    static UCmnUserSettingsSaveBase* LoadOrCreate(TSubclassOf<UCmnUserSettingsSaveBase> SaveClass, const FString& SlotName, int32 UserIndex);

    /**
     * SaveGameを保存する
     *
     * @param SaveObject 保存対象
     * @param SlotName スロット名
     * @param UserIndex ユーザーIndex
     * @return 保存成功ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Save")
    static bool Save(UCmnUserSettingsSaveBase* SaveObject, const FString& SlotName, int32 UserIndex);
};
