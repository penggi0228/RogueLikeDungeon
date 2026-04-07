// RldGridManager.cpp

#include "Game/Grid/RldGridManager.h"

#include "Common/Grid/CmnGridCoordFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldGridManager, Log, All);

/** コンストラクタ */
ARldGridManager::ARldGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

/** 開始時処理 */
void ARldGridManager::BeginPlay()
{
    Super::BeginPlay();
}

/** フロア定義を現在のグリッド状態へ反映する */
void ARldGridManager::ApplyFloorDefinition(const FRldFloorDefinition& floorDefinition)
{
    gridWidth = FMath::Max(1, floorDefinition.gridWidth);
    gridHeight = FMath::Max(1, floorDefinition.gridHeight);
    gridDefinition = floorDefinition.gridDefinition;
    wallCells = floorDefinition.wallCells;
    stairsGridCoord = floorDefinition.stairsGridCoord;
    bGenerateOuterWallsOnBeginPlay = floorDefinition.bGenerateOuterWalls;

    // 占有情報はフロア反映時にクリア
    occupantMap.Empty();

    // 必要時は外周壁を自動生成
    if (bGenerateOuterWallsOnBeginPlay)
    {
        GenerateOuterWallCells();
    }

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("ApplyFloorDefinition: グリッドサイズ=(%d,%d) 壁数=%d 階段座標=(%d,%d) 外周壁自動生成=%d"),
        gridWidth,
        gridHeight,
        wallCells.Num(),
        stairsGridCoord.X,
        stairsGridCoord.Y,
        bGenerateOuterWallsOnBeginPlay ? 1 : 0
    );
}

/**
 * 指定グリッド座標が有効範囲内か判定する
 */
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

/**
 * 指定グリッド座標が壁マスか判定する
 */
bool ARldGridManager::IsWallCell(const FIntPoint& gridCoord) const
{
    const bool bIsWallCell = wallCells.Contains(gridCoord);

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

/**
 * 指定グリッド座標が階段マスか判定する
 */
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

/**
 * 指定グリッド座標が占有されているか判定する
 */
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

/**
 * 指定グリッド座標へ通行可能か判定する
 */
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

    if (IsWallCell(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=0 理由=壁マス"),
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

/**
 * 指定グリッド座標の占有Actorを取得する
 */
AActor* ARldGridManager::GetOccupyingActor(const FIntPoint& gridCoord) const
{
    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(gridCoord);

    if (!foundActor)
    {
        return nullptr;
    }

    return foundActor->Get();
}

/**
 * グリッド座標へ占有Actorを登録する
 */
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

    if (IsWallCell(gridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("RegisterOccupant: 壁マスのため登録しません グリッド座標=(%d,%d)"), gridCoord.X, gridCoord.Y);
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

/**
 * グリッド座標から占有Actorを解除する
 */
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

/**
 * 占有Actorを別グリッド座標へ移動する
 */
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

    if (IsWallCell(toGridCoord))
    {
        UE_LOG(LogRldGridManager, Warning, TEXT("MoveOccupant: 移動先が壁マスのため移動しません 移動先=(%d,%d)"), toGridCoord.X, toGridCoord.Y);
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

/**
 * グリッド座標をワールド座標へ変換する
 */
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

/**
 * ワールド座標をグリッド座標へ変換する
 */
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

/**
 * 階段マス座標を設定する
 */
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

/** 必要に応じて外周壁を自動生成する */
void ARldGridManager::GenerateOuterWallCells()
{
    // グリッドサイズ不足時は外周壁を作れない
    if (gridWidth <= 0 || gridHeight <= 0)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("GenerateOuterWallCells: グリッドサイズ不正のため外周壁を生成しません サイズ=(%d,%d)"),
            gridWidth,
            gridHeight
        );
        return;
    }

    const int32 beforeWallCount = wallCells.Num();

    // 上端と下端を追加
    for (int32 x = 0; x < gridWidth; ++x)
    {
        AddWallCellUnique(FIntPoint(x, 0));
        AddWallCellUnique(FIntPoint(x, gridHeight - 1));
    }

    // 左端と右端を追加
    for (int32 y = 0; y < gridHeight; ++y)
    {
        AddWallCellUnique(FIntPoint(0, y));
        AddWallCellUnique(FIntPoint(gridWidth - 1, y));
    }

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("GenerateOuterWallCells: 外周壁を生成しました 追加前=%d 追加後=%d グリッドサイズ=(%d,%d)"),
        beforeWallCount,
        wallCells.Num(),
        gridWidth,
        gridHeight
    );
}

/**
 * 壁マスを重複なしで追加する
 */
void ARldGridManager::AddWallCellUnique(const FIntPoint& wallCoord)
{
    if (!wallCells.Contains(wallCoord))
    {
        wallCells.Add(wallCoord);
    }
}
