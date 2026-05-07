// CmnDebugWorldSubsystem.cpp

#include "Common/Debug/CmnDebugWorldSubsystem.h"

#include "Common/Debug/CmnDebugCategories.h"
#include "Common/Debug/CmnDebugDrawLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnDebugWorldSubsystem, Log, All);

/** Subsystem生成可否を判定する */
bool UCmnDebugWorldSubsystem::ShouldCreateSubsystem(UObject* outer) const
{
    return Super::ShouldCreateSubsystem(outer);
}

/** Subsystem初期化時にカテゴリ状態を初期化する */
void UCmnDebugWorldSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
    Super::Initialize(collection);

    InitializeDefaultCategories();
}

/** デバッグ描画全体の有効状態を設定する */
void UCmnDebugWorldSubsystem::SetDebugEnabled(bool bEnabled)
{
    if (bDebugEnabled == bEnabled)
    {
        return;
    }

    bDebugEnabled = bEnabled;

    UE_LOG(
        LogCmnDebugWorldSubsystem,
        Log,
        TEXT("SetDebugEnabled: デバッグ描画全体=%s"),
        bDebugEnabled ? TEXT("有効") : TEXT("無効")
    );
}

/** デバッグカテゴリの有効状態を設定する */
void UCmnDebugWorldSubsystem::SetCategoryEnabled(FName categoryName, bool bEnabled)
{
    if (categoryName.IsNone())
    {
        UE_LOG(
            LogCmnDebugWorldSubsystem,
            Warning,
            TEXT("SetCategoryEnabled: カテゴリ名がNoneのため更新しません")
        );
        return;
    }

    const bool bCurrentEnabled = IsCategoryEnabled(categoryName);

    if (bCurrentEnabled == bEnabled && categoryEnabledMap.Contains(categoryName))
    {
        return;
    }

    categoryEnabledMap.FindOrAdd(categoryName) = bEnabled;

    UE_LOG(
        LogCmnDebugWorldSubsystem,
        Log,
        TEXT("SetCategoryEnabled: カテゴリ=%s 状態=%s"),
        *categoryName.ToString(),
        bEnabled ? TEXT("有効") : TEXT("無効")
    );
}

/** デバッグカテゴリが有効か判定する */
bool UCmnDebugWorldSubsystem::IsCategoryEnabled(FName categoryName) const
{
    if (categoryName.IsNone())
    {
        return false;
    }

    const bool* foundEnabled = categoryEnabledMap.Find(categoryName);

    if (!foundEnabled)
    {
        return false;
    }

    return *foundEnabled;
}

/** 単一グリッドマスを描画する */
void UCmnDebugWorldSubsystem::DrawGridCell(
    const FCmnGridDefinition& gridDefinition,
    const FIntPoint& gridCoord,
    const FCmnDebugDrawStyle& drawStyle
) const
{
    if (!CanDrawDebug())
    {
        return;
    }

    UCmnDebugDrawLibrary::DrawGridCell(
        GetWorld(),
        gridDefinition,
        gridCoord,
        drawStyle
    );
}

/** 複数グリッドマスを描画する */
void UCmnDebugWorldSubsystem::DrawGridCells(
    const FCmnGridDefinition& gridDefinition,
    const TArray<FIntPoint>& gridCoords,
    const FCmnDebugDrawStyle& drawStyle
) const
{
    if (!CanDrawDebug())
    {
        return;
    }

    UCmnDebugDrawLibrary::DrawGridCells(
        GetWorld(),
        gridDefinition,
        gridCoords,
        drawStyle
    );
}

/** 部屋矩形外枠を描画する */
void UCmnDebugWorldSubsystem::DrawGridRoomBounds(
    const FCmnGridDefinition& gridDefinition,
    const FCmnGridRoom& room,
    const FCmnDebugDrawStyle& drawStyle
) const
{
    if (!CanDrawDebug())
    {
        return;
    }

    UCmnDebugDrawLibrary::DrawGridRoomBounds(
        GetWorld(),
        gridDefinition,
        room,
        drawStyle
    );
}

/** デバッグ描画を実行可能か判定する */
bool UCmnDebugWorldSubsystem::CanDrawDebug() const
{
    if (!bDebugEnabled)
    {
        return false;
    }

    if (!GetWorld())
    {
        UE_LOG(
            LogCmnDebugWorldSubsystem,
            Warning,
            TEXT("CanDrawDebug: World未取得のため描画しません")
        );
        return false;
    }

    return true;
}

/** デフォルトのデバッグカテゴリ状態を初期化する */
void UCmnDebugWorldSubsystem::InitializeDefaultCategories()
{
    categoryEnabledMap.Reset();

    categoryEnabledMap.Add(CmnDebugCategories::Grid, true);
    categoryEnabledMap.Add(CmnDebugCategories::Floor, true);
    categoryEnabledMap.Add(CmnDebugCategories::AI, false);
    categoryEnabledMap.Add(CmnDebugCategories::Collision, false);

    UE_LOG(
        LogCmnDebugWorldSubsystem,
        Log,
        TEXT("InitializeDefaultCategories: デバッグカテゴリを初期化しました Grid=%d Floor=%d AI=%d Collision=%d"),
        IsCategoryEnabled(CmnDebugCategories::Grid) ? 1 : 0,
        IsCategoryEnabled(CmnDebugCategories::Floor) ? 1 : 0,
        IsCategoryEnabled(CmnDebugCategories::AI) ? 1 : 0,
        IsCategoryEnabled(CmnDebugCategories::Collision) ? 1 : 0
    );
}
