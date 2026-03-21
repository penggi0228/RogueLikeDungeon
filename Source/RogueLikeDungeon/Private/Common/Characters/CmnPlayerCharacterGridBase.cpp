// CmnPlayerCharacterGridBase.cpp

#include "Common/Characters/CmnPlayerCharacterGridBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnPlayerCharacterGridBase, Log, All);

/** コンストラクタ */
ACmnPlayerCharacterGridBase::ACmnPlayerCharacterGridBase()
{
    // 現段階ではTick不要
    PrimaryActorTick.bCanEverTick = false;
}

/**
 * 開始時に現在のグリッド座標をActor位置へ反映する
 */
void ACmnPlayerCharacterGridBase::BeginPlay()
{
    Super::BeginPlay();

    // PlayerStartなどで配置された現在位置を基準に、初期グリッド座標を確定する
    CurrentGridCoord = WorldToGrid(GetActorLocation());

    UE_LOG(
        LogCmnPlayerCharacterGridBase,
        Log,
        TEXT("BeginPlay: InitialWorld(X=%f Y=%f Z=%f) -> InitialGrid(%d, %d)"),
        GetActorLocation().X,
        GetActorLocation().Y,
        GetActorLocation().Z,
        CurrentGridCoord.X,
        CurrentGridCoord.Y
    );
}

/**
 * グリッド座標をワールド座標へ変換する
 *
 * 今は
 * Grid X = World X
 * Grid Y = World Y
 * として単純変換する
 */
FVector ACmnPlayerCharacterGridBase::GridToWorld(const FIntPoint& GridCoord) const
{
    return FVector(
        static_cast<float>(GridCoord.X) * CellSize,
        static_cast<float>(GridCoord.Y) * CellSize,
        GroundZ
    );
}

/**
 * ワールド座標をグリッド座標へ変換する
 *
 * 今は
 * World X =  GridX
 * World Y = Grid Y
 * として単純変換する
 */
FIntPoint ACmnPlayerCharacterGridBase::WorldToGrid(const FVector& WorldLocation) const
{
    const int32 GridX = FMath::RoundToInt(WorldLocation.X / CellSize);
    const int32 GridY = FMath::RoundToInt(WorldLocation.Y / CellSize);

    return FIntPoint(GridX, GridY);
}

/**
 * 現在のグリッド座標を設定する
 */
void ACmnPlayerCharacterGridBase::SetCurrentGridCoord(const FIntPoint& NewGridCoord)
{
    CurrentGridCoord = NewGridCoord;

    UE_LOG(
        LogCmnPlayerCharacterGridBase,
        Log,
        TEXT("SetCurrentGridCoord: Grid(%d, %d)"),
        CurrentGridCoord.X,
        CurrentGridCoord.Y
    );

    SyncActorLocationFromGridCoord();
}

/**
 * 現在のグリッド座標をActor位置へ反映する
 */
void ACmnPlayerCharacterGridBase::SyncActorLocationFromGridCoord()
{
    const FVector NewWorldLocation = GridToWorld(CurrentGridCoord);

    SetActorLocation(NewWorldLocation);

    UE_LOG(
        LogCmnPlayerCharacterGridBase,
        Log,
        TEXT("SyncActorLocationFromGridCoord: Grid(%d, %d) -> World(X=%f Y=%f Z=%f)"),
        CurrentGridCoord.X,
        CurrentGridCoord.Y,
        NewWorldLocation.X,
        NewWorldLocation.Y,
        NewWorldLocation.Z
    );
}
