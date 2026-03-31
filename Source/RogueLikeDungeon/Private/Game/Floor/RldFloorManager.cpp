// RldFloorManager.cpp

#include "Game/Floor/RldFloorManager.h"

#include "Kismet/GameplayStatics.h"

#include "Game/Characters/RldPlayerCharacter.h"
#include "Game/Grid/RldGridManager.h"
#include "Game/Turn/RldTurnManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldFloorManager, Log, All);

/** フロア管理Actorを初期化する */
ARldFloorManager::ARldFloorManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

/** 開始時処理 */
void ARldFloorManager::BeginPlay()
{
    Super::BeginPlay();

    // 各管理Actorを取得
    ResolveGridManager();
    ResolvePlayerCharacter();
    ResolveTurnManager();

    if (bStartOnBeginPlay)
    {
        StartFloor();
    }
}

/** フロア開始処理を行う */
void ARldFloorManager::StartFloor()
{
    StartFloorAt(currentFloorIndex);
}

/** 指定フロアの開始処理を行う */
void ARldFloorManager::StartFloorAt(int32 floorIndex)
{
    currentFloorIndex = FMath::Max(1, floorIndex);

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("StartFloor: フロア番号=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        currentFloorIndex,
        playerStartGridCoord.X,
        playerStartGridCoord.Y,
        stairsGridCoord.X,
        stairsGridCoord.Y
    );

    ApplyFloorState();
}

/** 次フロアへ進む */
void ARldFloorManager::GoToNextFloor()
{
    ++currentFloorIndex;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("GoToNextFloor: 次フロア番号=%d"),
        currentFloorIndex
    );

    StartFloor();
}

/** プレイヤー開始座標を設定する */
void ARldFloorManager::SetPlayerStartGridCoord(const FIntPoint& newGridCoord)
{
    playerStartGridCoord = newGridCoord;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("SetPlayerStartGridCoord: 開始座標=(%d,%d)"),
        playerStartGridCoord.X,
        playerStartGridCoord.Y
    );
}

/** 階段座標を設定する */
void ARldFloorManager::SetStairsGridCoord(const FIntPoint& newGridCoord)
{
    stairsGridCoord = newGridCoord;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("SetStairsGridCoord: 階段座標=(%d,%d)"),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );
}

/** グリッド管理Actorを取得する */
void ARldFloorManager::ResolveGridManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldGridManager::StaticClass());
    gridManager = Cast<ARldGridManager>(foundActor);

    if (!gridManager)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ResolveGridManager: ARldGridManagerがレベル上に見つからない")
        );
        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("ResolveGridManager: 名前=%s クラス=%s"),
        *gridManager->GetName(),
        *gridManager->GetClass()->GetName()
    );
}

/** プレイヤーキャラクターを取得する */
void ARldFloorManager::ResolvePlayerCharacter()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldPlayerCharacter::StaticClass());
    playerCharacter = Cast<ARldPlayerCharacter>(foundActor);

    if (!playerCharacter)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ResolvePlayerCharacter: ARldPlayerCharacterがレベル上に見つからない")
        );
        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("ResolvePlayerCharacter: 名前=%s クラス=%s"),
        *playerCharacter->GetName(),
        *playerCharacter->GetClass()->GetName()
    );
}

/** ターン管理Actorを取得する */
void ARldFloorManager::ResolveTurnManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldTurnManager::StaticClass());
    turnManager = Cast<ARldTurnManager>(foundActor);

    if (!turnManager)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ResolveTurnManager: ARldTurnManagerがレベル上に見つからない")
        );
        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("ResolveTurnManager: 名前=%s クラス=%s 現在ターン数=%d"),
        *turnManager->GetName(),
        *turnManager->GetClass()->GetName(),
        turnManager->GetCurrentTurnIndex()
    );
}

/** フロア状態をグリッドとプレイヤーへ反映する */
void ARldFloorManager::ApplyFloorState()
{
    // GridManager未取得時は反映しない
    if (!gridManager)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ApplyFloorState: GridManager未取得のため反映できない")
        );
        return;
    }

    // PlayerCharacter未取得時は反映しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ApplyFloorState: PlayerCharacter未取得のため反映できない")
        );
        return;
    }

    // GridManagerへ階段座標を反映
    gridManager->SetStairsGridCoord(stairsGridCoord);

    // プレイヤー開始座標を反映
    playerCharacter->SetCurrentGridCoord(playerStartGridCoord);
    playerCharacter->SetActorLocation(gridManager->GridToWorld(playerStartGridCoord));

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("ApplyFloorState: 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        playerStartGridCoord.X,
        playerStartGridCoord.Y,
        stairsGridCoord.X,
        stairsGridCoord.Y
    );

    // フロア開始時はターンを初期状態へ戻す
    if (turnManager)
    {
        turnManager->ResetTurn();
    }
}
