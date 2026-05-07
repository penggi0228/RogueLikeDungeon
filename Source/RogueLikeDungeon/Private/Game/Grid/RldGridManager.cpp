// RldGridManager.cpp

#include "Game/Grid/RldGridManager.h"

#include "Common/Debug/CmnDebugCategories.h"
#include "Common/Debug/CmnDebugWorldSubsystem.h"
#include "Common/Grid/CmnGridCoordFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldGridManager, Log, All);

/** コンストラクタ */
ARldGridManager::ARldGridManager()
{
    PrimaryActorTick.bCanEverTick = true;

    floorDebugStyle.drawColor = FColor::Green;
    floorDebugStyle.bPersistentLines = false;
    floorDebugStyle.duration = 0.15f;
    floorDebugStyle.lineThickness = 1.5f;
    floorDebugStyle.zOffset = 5.0f;
    floorDebugStyle.sizeScale = 0.18f;

    wallDebugStyle.drawColor = FColor::Red;
    wallDebugStyle.bPersistentLines = false;
    wallDebugStyle.duration = 0.15f;
    wallDebugStyle.lineThickness = 1.5f;
    wallDebugStyle.zOffset = 25.0f;
    wallDebugStyle.sizeScale = 0.45f;

    stairsDebugStyle.drawColor = FColor::Blue;
    stairsDebugStyle.bPersistentLines = false;
    stairsDebugStyle.duration = 0.15f;
    stairsDebugStyle.lineThickness = 1.5f;
    stairsDebugStyle.zOffset = 45.0f;
    stairsDebugStyle.sizeScale = 0.30f;
}

/** 開始時処理 */
void ARldGridManager::BeginPlay()
{
    Super::BeginPlay();
}

/** 毎フレーム処理 */
void ARldGridManager::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);

    UpdateContinuousDebugDraw(deltaSeconds);
}

/** フロア定義と生成結果を現在のグリッド状態へ反映する */
void ARldGridManager::ApplyFloorLayout(
    const FRldFloorDefinition& floorDefinition,
    const FCmnGridLayoutBuildResult& floorLayout
)
{
    gridWidth = FMath::Max(1, floorLayout.gridWidth);
    gridHeight = FMath::Max(1, floorLayout.gridHeight);
    gridDefinition = floorDefinition.gridDefinition;

    floorCells = floorLayout.floorCells;
    wallCells = floorLayout.wallCells;
    stairsGridCoord = floorLayout.stairsGridCoord;

    RebuildCellSets();

    // フロア反映時は占有情報を初期化
    occupantMap.Empty();

    // 常時デバッグ描画の状態を初期化
    continuousDebugDrawElapsed = 0.0f;
    bDebugLegendLogged = false;

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("ApplyFloorLayout: グリッドサイズ=(%d,%d) 床数=%d 壁数=%d 階段座標=(%d,%d)"),
        gridWidth,
        gridHeight,
        floorCells.Num(),
        wallCells.Num(),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );

    // フロア反映直後の手動描画としてログありで描画する
    if (bDrawDebugOnApplyFloorLayout)
    {
        DrawDebugGridState();
        LogDebugDrawLegend();
    }
}

/** 指定グリッド座標が有効範囲内か判定する */
bool ARldGridManager::IsInsideGrid(const FIntPoint& gridCoord) const
{
    const bool bIsInsideX = (gridCoord.X >= 0) && (gridCoord.X < gridWidth);
    const bool bIsInsideY = (gridCoord.Y >= 0) && (gridCoord.Y < gridHeight);
    const bool bIsInsideGrid = bIsInsideX && bIsInsideY;

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsInsideGrid: グリッド座標=(%d,%d) グリッドサイズ=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        gridWidth,
        gridHeight,
        bIsInsideGrid ? 1 : 0
    );

    return bIsInsideGrid;
}

/** 指定グリッド座標が床マスか判定する */
bool ARldGridManager::IsFloorCell(const FIntPoint& gridCoord) const
{
    const bool bIsFloor = floorCellSet.Contains(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsFloorCell: グリッド座標=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        bIsFloor ? 1 : 0
    );

    return bIsFloor;
}

/** 指定グリッド座標が壁マスか判定する */
bool ARldGridManager::IsWallCell(const FIntPoint& gridCoord) const
{
    const bool bIsWallCell = wallCellSet.Contains(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWallCell: グリッド座標=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        bIsWallCell ? 1 : 0
    );

    return bIsWallCell;
}

/** 指定グリッド座標が階段マスか判定する */
bool ARldGridManager::IsStairsCell(const FIntPoint& gridCoord) const
{
    const bool bIsStairsCell = (gridCoord == stairsGridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsStairsCell: グリッド座標=(%d,%d) 階段座標=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        stairsGridCoord.X,
        stairsGridCoord.Y,
        bIsStairsCell ? 1 : 0
    );

    return bIsStairsCell;
}

/** 指定グリッド座標が占有されているか判定する */
bool ARldGridManager::IsOccupied(const FIntPoint& gridCoord) const
{
    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(gridCoord);
    const bool bIsOccupied = (foundActor != nullptr) && foundActor->IsValid();

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsOccupied: グリッド座標=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        bIsOccupied ? 1 : 0
    );

    return bIsOccupied;
}

/** 指定グリッド座標へ通行可能か判定する */
bool ARldGridManager::IsWalkable(const FIntPoint& gridCoord) const
{
    if (!IsInsideGrid(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=0 理由=範囲外"),
            gridCoord.X,
            gridCoord.Y
        );
        return false;
    }

    if (!IsFloorCell(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=0 理由=床マスではない"),
            gridCoord.X,
            gridCoord.Y
        );
        return false;
    }

    if (IsOccupied(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=0 理由=占有中"),
            gridCoord.X,
            gridCoord.Y
        );
        return false;
    }

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=1"),
        gridCoord.X,
        gridCoord.Y
    );

    return true;
}

/** 指定グリッド座標の占有Actorを取得する */
AActor* ARldGridManager::GetOccupyingActor(const FIntPoint& gridCoord) const
{
    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(gridCoord);

    if (!foundActor)
    {
        return nullptr;
    }

    return foundActor->Get();
}

/** グリッド座標へ占有Actorを登録する */
bool ARldGridManager::RegisterOccupant(const FIntPoint& gridCoord, AActor* occupantActor)
{
    if (!occupantActor)
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("RegisterOccupant: occupantActorがnullのため登録しません"));
        return false;
    }

    if (!IsInsideGrid(gridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("RegisterOccupant: 範囲外のため登録しません グリッド座標=(%d,%d)"), gridCoord.X, gridCoord.Y);
        return false;
    }

    if (!IsFloorCell(gridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("RegisterOccupant: 床マスではないため登録しません グリッド座標=(%d,%d)"), gridCoord.X, gridCoord.Y);
        return false;
    }

    if (IsOccupied(gridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("RegisterOccupant: すでに占有中のため登録しません グリッド座標=(%d,%d)"), gridCoord.X, gridCoord.Y);
        return false;
    }

    occupantMap.Add(gridCoord, occupantActor);

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("RegisterOccupant: 登録しました グリッド座標=(%d,%d) Actor=%s"),
        gridCoord.X,
        gridCoord.Y,
        *occupantActor->GetName()
    );

    return true;
}

/** グリッド座標から占有Actorを解除する */
bool ARldGridManager::UnregisterOccupant(const FIntPoint& gridCoord, AActor* occupantActor)
{
    if (!occupantActor)
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("UnregisterOccupant: occupantActorがnullのため解除しません"));
        return false;
    }

    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(gridCoord);

    if (!foundActor || !foundActor->IsValid())
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("UnregisterOccupant: 登録が存在しないため解除しません グリッド座標=(%d,%d)"), gridCoord.X, gridCoord.Y);
        return false;
    }

    if (foundActor->Get() != occupantActor)
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("UnregisterOccupant: 登録Actorが一致しないため解除しません グリッド座標=(%d,%d)"), gridCoord.X, gridCoord.Y);
        return false;
    }

    occupantMap.Remove(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("UnregisterOccupant: 解除しました グリッド座標=(%d,%d) Actor=%s"),
        gridCoord.X,
        gridCoord.Y,
        *occupantActor->GetName()
    );

    return true;
}

/** 占有Actorを別グリッド座標へ移動する */
bool ARldGridManager::MoveOccupant(const FIntPoint& fromGridCoord, const FIntPoint& toGridCoord, AActor* occupantActor)
{
    if (!occupantActor)
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("MoveOccupant: occupantActorがnullのため移動しません"));
        return false;
    }

    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(fromGridCoord);

    if (!foundActor || !foundActor->IsValid() || foundActor->Get() != occupantActor)
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("MoveOccupant: 移動前登録が不正のため移動しません 移動前=(%d,%d)"), fromGridCoord.X, fromGridCoord.Y);
        return false;
    }

    if (!IsInsideGrid(toGridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("MoveOccupant: 移動先が範囲外のため移動しません 移動先=(%d,%d)"), toGridCoord.X, toGridCoord.Y);
        return false;
    }

    if (!IsFloorCell(toGridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("MoveOccupant: 移動先が床マスではないため移動しません 移動先=(%d,%d)"), toGridCoord.X, toGridCoord.Y);
        return false;
    }

    if (IsOccupied(toGridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("MoveOccupant: 移動先が占有中のため移動しません 移動先=(%d,%d)"), toGridCoord.X, toGridCoord.Y);
        return false;
    }

    occupantMap.Remove(fromGridCoord);
    occupantMap.Add(toGridCoord, occupantActor);

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("MoveOccupant: 移動しました 移動前=(%d,%d) 移動先=(%d,%d) Actor=%s"),
        fromGridCoord.X,
        fromGridCoord.Y,
        toGridCoord.X,
        toGridCoord.Y,
        *occupantActor->GetName()
    );

    return true;
}

/** すべての占有情報をクリアする */
void ARldGridManager::ClearAllOccupants()
{
    const int32 previousOccupantCount = occupantMap.Num();

    occupantMap.Empty();

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("ClearAllOccupants: 占有情報をクリアしました 件数=%d"),
        previousOccupantCount
    );
}

/** グリッド座標をワールド座標へ変換する */
FVector ARldGridManager::GridToWorld(const FIntPoint& gridCoord) const
{
    const FVector worldLocation = UCmnGridCoordFunctionLibrary::GridToWorld(gridDefinition, gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("GridToWorld: グリッド座標=(%d,%d) ワールド座標=(%f,%f,%f)"),
        gridCoord.X,
        gridCoord.Y,
        worldLocation.X,
        worldLocation.Y,
        worldLocation.Z
    );

    return worldLocation;
}

/** ワールド座標をグリッド座標へ変換する */
FIntPoint ARldGridManager::WorldToGrid(const FVector& worldLocation) const
{
    const FIntPoint gridCoord = UCmnGridCoordFunctionLibrary::WorldToGrid(gridDefinition, worldLocation);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("WorldToGrid: ワールド座標=(%f,%f,%f) グリッド座標=(%d,%d)"),
        worldLocation.X,
        worldLocation.Y,
        worldLocation.Z,
        gridCoord.X,
        gridCoord.Y
    );

    return gridCoord;
}

/** 現在のフロア状態をデバッグ描画する */
void ARldGridManager::DrawDebugGridState() const
{
    DrawDebugGridStateInternal(true);
}

/** デバッグ描画凡例をログ出力する */
void ARldGridManager::LogDebugDrawLegend()
{
    if (bDebugLegendLogged)
    {
        return;
    }

    bDebugLegendLogged = true;

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("DebugDrawLegend: 緑=床マス 赤=壁マス 青=階段マス")
    );

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("DebugDrawLegend: 床数=%d 壁数=%d 階段座標=(%d,%d)"),
        floorCells.Num(),
        wallCells.Num(),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );
}

/** 階段マス座標を設定する */
void ARldGridManager::SetStairsGridCoord(const FIntPoint& newGridCoord)
{
    stairsGridCoord = newGridCoord;

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("SetStairsGridCoord: 階段座標=(%d,%d)"),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );
}

/** 配列から床マスと壁マスの検索用Setを再構築する */
void ARldGridManager::RebuildCellSets()
{
    floorCellSet.Reset();
    wallCellSet.Reset();

    for (const FIntPoint& floorCell : floorCells)
    {
        floorCellSet.Add(floorCell);
    }

    for (const FIntPoint& wallCell : wallCells)
    {
        wallCellSet.Add(wallCell);
    }
}

/** グリッドデバッグ描画を更新する */
void ARldGridManager::UpdateContinuousDebugDraw(float deltaSeconds)
{
    if (!ShouldDrawContinuousDebug())
    {
        continuousDebugDrawElapsed = 0.0f;
        return;
    }

    continuousDebugDrawElapsed += deltaSeconds;

    if (continuousDebugDrawElapsed < continuousDebugDrawInterval)
    {
        return;
    }

    continuousDebugDrawElapsed = 0.0f;

    // 常時描画ではログを出さない
    DrawDebugGridStateInternal(false);
}

/** グリッド常時デバッグ描画が有効か判定する */
bool ARldGridManager::ShouldDrawContinuousDebug() const
{
    if (!bEnableContinuousDebugDraw)
    {
        return false;
    }

    UWorld* world = GetWorld();

    if (!world)
    {
        return false;
    }

    const UCmnDebugWorldSubsystem* debugSubsystem = world->GetSubsystem<UCmnDebugWorldSubsystem>();

    if (!debugSubsystem)
    {
        return false;
    }

    return debugSubsystem->IsDebugEnabled()
        && debugSubsystem->IsCategoryEnabled(CmnDebugCategories::Grid);
}

/** 現在のフロア状態をデバッグ描画する */
void ARldGridManager::DrawDebugGridStateInternal(bool bOutputLog) const
{
    UWorld* world = GetWorld();

    // World未取得時は描画しない
    if (!world)
    {
        if (bOutputLog)
        {
            UE_LOG(
                LogRldGridManager,
                Warning,
                TEXT("DrawDebugGridState: World未取得のため描画しません")
            );
        }

        return;
    }

    UCmnDebugWorldSubsystem* debugSubsystem = world->GetSubsystem<UCmnDebugWorldSubsystem>();

    // DebugSubsystem未取得時は描画しない
    if (!debugSubsystem)
    {
        if (bOutputLog)
        {
            UE_LOG(
                LogRldGridManager,
                Warning,
                TEXT("DrawDebugGridState: DebugSubsystem未取得のため描画しません")
            );
        }

        return;
    }

    // デバッグ描画全体またはGridカテゴリが無効な場合は描画しない
    if (!debugSubsystem->IsDebugEnabled() || !debugSubsystem->IsCategoryEnabled(CmnDebugCategories::Grid))
    {
        return;
    }

    // 床マスを描画
    debugSubsystem->DrawGridCells(
        gridDefinition,
        floorCells,
        floorDebugStyle
    );

    // 壁マスを描画
    debugSubsystem->DrawGridCells(
        gridDefinition,
        wallCells,
        wallDebugStyle
    );

    // 階段マスを描画
    debugSubsystem->DrawGridCell(
        gridDefinition,
        stairsGridCoord,
        stairsDebugStyle
    );

    if (bOutputLog)
    {
        UE_LOG(
            LogRldGridManager,
            Log,
            TEXT("DrawDebugGridState: 床数=%d 壁数=%d 階段座標=(%d,%d)"),
            floorCells.Num(),
            wallCells.Num(),
            stairsGridCoord.X,
            stairsGridCoord.Y
        );
    }
}
