// CmnSaveGameLibrary.cpp

#include "Common/Save/CmnSaveGameLibrary.h"

#include "Common/Save/CmnUserSettingsSaveBase.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnSave, Log, All);

/**
 * スロットからSaveGameをロードする。
 * 存在すればロード
 * 存在しない/ロード失敗/キャスト失敗時は新規作成
 *
 * @param SaveClass 作成対象のSaveGameクラス
 * @param SlotName  スロット名
 * @param UserIndex ユーザーIndex
 * @return 読み込みまたは新規作成したSaveGame(失敗時はnullptr)
 */
UCmnUserSettingsSaveBase* UCmnSaveGameLibrary::LoadOrCreate(
    TSubclassOf<UCmnUserSettingsSaveBase> SaveClass,
    const FString& SlotName,
    int32 UserIndex)
{
    // SaveClassが無効なら何もしない
    if (!*SaveClass)
    {
        UE_LOG(LogCmnSave, Warning, TEXT("LoadOrCreate: SaveClass is invalid. Slot=%s"), *SlotName);
        return nullptr;
    }

    UCmnUserSettingsSaveBase* Result = nullptr;

    // 既存スロットが存在する場合はロード試行
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);

        // ロードが成功している前提で、想定クラスへキャスト
        Result = Cast<UCmnUserSettingsSaveBase>(Loaded);

        // ロード自体はできたが、型が違う(またはクラス差し替え後)などでキャストに失敗した場合
        if (Loaded && !Result)
        {
            UE_LOG(LogCmnSave, Warning,
                TEXT("LoadOrCreate: Cast failed. Slot=%s LoadedClass=%s ExpectedBase=%s"),
                *SlotName,
                *Loaded->GetClass()->GetName(),
                *UCmnUserSettingsSaveBase::StaticClass()->GetName());
        }
    }

    // ロード失敗/スロット無し/キャスト失敗の場合は新規作成
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
 * SaveGameをスロットへ保存する。
 * ・保存前にSavedAtを更新する
 *
 * @param SaveObject 保存対象
 * @param SlotName   スロット名
 * @param UserIndex  ユーザーIndex
 * @return 保存成功時true
 */
bool UCmnSaveGameLibrary::Save(
    UCmnUserSettingsSaveBase* SaveObject,
    const FString& SlotName,
    int32 UserIndex)
{
    // 保存対象が無効なら失敗
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