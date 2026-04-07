// CmnSaveGameLibrary.cpp

#include "Common/Save/CmnSaveGameLibrary.h"

#include "Common/Save/CmnUserSettingsSaveBase.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnSave, Log, All);

/**
 * SaveGameをロードまたは新規作成する
 *
 * @param SaveClass SaveGameクラス
 * @param SlotName スロット名
 * @param UserIndex ユーザーIndex
 * @return ロードまたは新規作成したSaveGame
 */
UCmnUserSettingsSaveBase* UCmnSaveGameLibrary::LoadOrCreate(
    TSubclassOf<UCmnUserSettingsSaveBase> SaveClass,
    const FString& SlotName,
    int32 UserIndex)
{
    // SaveGameクラスが無効な場合はnullptrを返す
    if (!*SaveClass)
    {
        UE_LOG(LogCmnSave, Warning, TEXT("LoadOrCreate: SaveClass is invalid. Slot=%s"), *SlotName);
        return nullptr;
    }

    UCmnUserSettingsSaveBase* Result = nullptr;

    // 既存スロットがある場合はロードを試行
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);

        // ロードしたSaveGameを共通ベース型へ変換
        Result = Cast<UCmnUserSettingsSaveBase>(Loaded);

        // ロード済みオブジェクトの型が一致しない場合は警告を出す
        if (Loaded && !Result)
        {
            UE_LOG(LogCmnSave, Warning,
                TEXT("LoadOrCreate: Cast failed. Slot=%s LoadedClass=%s ExpectedBase=%s"),
                *SlotName,
                *Loaded->GetClass()->GetName(),
                *UCmnUserSettingsSaveBase::StaticClass()->GetName());
        }
    }

    // ロードできなかった場合は新規作成
    if (!Result)
    {
        Result = Cast<UCmnUserSettingsSaveBase>(UGameplayStatics::CreateSaveGameObject(SaveClass));

        if (!Result)
        {
            UE_LOG(LogCmnSave, Warning,
                TEXT("LoadOrCreate: CreateSaveGameObject failed. Slot=%s SaveClass=%s"),
                *SlotName,
                *SaveClass->GetName());
        }
    }

    return Result;
}

/**
 * SaveGameを保存する
 *
 * @param SaveObject 保存対象
 * @param SlotName スロット名
 * @param UserIndex ユーザーIndex
 * @return 保存成功ならtrue
 */
bool UCmnSaveGameLibrary::Save(
    UCmnUserSettingsSaveBase* SaveObject,
    const FString& SlotName,
    int32 UserIndex)
{
    // 保存対象が無効な場合は失敗として返す
    if (!SaveObject)
    {
        UE_LOG(LogCmnSave, Warning, TEXT("Save: SaveObject is null. Slot=%s"), *SlotName);
        return false;
    }

    // 保存時刻を更新
    SaveObject->SavedAt = FDateTime::Now();

    // スロットへ保存
    const bool bOK = UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, UserIndex);

    if (!bOK)
    {
        UE_LOG(LogCmnSave, Warning,
            TEXT("Save: SaveGameToSlot failed. Slot=%s UserIndex=%d Class=%s"),
            *SlotName,
            UserIndex,
            *SaveObject->GetClass()->GetName());
    }

    return bOK;
}
