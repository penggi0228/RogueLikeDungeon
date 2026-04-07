// RldFloorManager.cpp

#include "Game/Floor/RldFloorManager.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

#include "Game/Characters/RldPlayerCharacter.h"
#include "Game/Enemies/RldEnemyManager.h"
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

    ResolveManagers();

    if (bStartOnBeginPlay)
    {
        StartFloor();
    }
}

/** 現在のフロア番号でフロア開始処理を行う */
void ARldFloorManager::StartFloor()
{
    StartFloorAt(currentFloorIndex);
}

/** 指定フロア番号でフロア開始処理を行う */
void ARldFloorManager::StartFloorAt(int32 floorIndex)
{
    const int32 targetFloorIndex = FMath::Max(1, floorIndex);
    FRldFloorDefinition loadedFloorDefinition;

    // フロア定義読込失敗時は開始しない
    if (!TryLoadFloorDefinition(targetFloorIndex, loadedFloorDefinition))
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("StartFloorAt: フロア定義読込に失敗したため開始しません フロア番号=%d"),
            targetFloorIndex
        );
        return;
    }

    currentFloorIndex = targetFloorIndex;
    currentFloorDefinition = loadedFloorDefinition;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("StartFloorAt: フロア番号=%d グリッドサイズ=(%d,%d) 開始座標=(%d,%d) 階段座標=(%d,%d) 壁数=%d"),
        currentFloorIndex,
        currentFloorDefinition.gridWidth,
        currentFloorDefinition.gridHeight,
        currentFloorDefinition.playerStartGridCoord.X,
        currentFloorDefinition.playerStartGridCoord.Y,
        currentFloorDefinition.stairsGridCoord.X,
        currentFloorDefinition.stairsGridCoord.Y,
        currentFloorDefinition.wallCells.Num()
    );

    ApplyFloorState();
}

/** 次フロアへ進む */
void ARldFloorManager::GoToNextFloor()
{
    const int32 nextFloorIndex = currentFloorIndex + 1;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("GoToNextFloor: 次フロア番号=%d"),
        nextFloorIndex
    );

    StartFloorAt(nextFloorIndex);
}

/** 管理Actor群を取得する */
void ARldFloorManager::ResolveManagers()
{
    ResolveGridManager();
    ResolvePlayerCharacter();
    ResolveTurnManager();
    ResolveEnemyManager();
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

/** エネミー管理Actorを取得する */
void ARldFloorManager::ResolveEnemyManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldEnemyManager::StaticClass());
    enemyManager = Cast<ARldEnemyManager>(foundActor);

    if (!enemyManager)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ResolveEnemyManager: ARldEnemyManagerがレベル上に見つからない")
        );
        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("ResolveEnemyManager: 名前=%s クラス=%s エネミー数=%d"),
        *enemyManager->GetName(),
        *enemyManager->GetClass()->GetName(),
        enemyManager->GetEnemyCount()
    );
}

/**
 * 指定フロア番号に対応するRowNameを作成する
 */
FName ARldFloorManager::BuildFloorRowName(int32 floorIndex) const
{
    return FName(*FString::Printf(TEXT("Floor_%03d"), floorIndex));
}

/**
 * 指定フロア番号の定義を読込する
 */
bool ARldFloorManager::TryLoadFloorDefinition(int32 floorIndex, FRldFloorDefinition& outFloorDefinition) const
{
    // DataTable未設定時は読込失敗
    if (!floorDefinitionDataTable)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("TryLoadFloorDefinition: floorDefinitionDataTableがnullのため読込に失敗しました")
        );
        return false;
    }

    const FName rowName = BuildFloorRowName(floorIndex);
    static const FString contextString = TEXT("RldFloorDefinitionLookup");

    const FRldFloorDefinition* foundDefinition = floorDefinitionDataTable->FindRow<FRldFloorDefinition>(
        rowName,
        contextString,
        true
    );

    // 対応行未取得時は読込失敗
    if (!foundDefinition)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("TryLoadFloorDefinition: 対応行が存在しないため読込に失敗しました RowName=%s"),
            *rowName.ToString()
        );
        return false;
    }

    outFloorDefinition = *foundDefinition;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("TryLoadFloorDefinition: 読込しました RowName=%s グリッドサイズ=(%d,%d) 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        *rowName.ToString(),
        outFloorDefinition.gridWidth,
        outFloorDefinition.gridHeight,
        outFloorDefinition.playerStartGridCoord.X,
        outFloorDefinition.playerStartGridCoord.Y,
        outFloorDefinition.stairsGridCoord.X,
        outFloorDefinition.stairsGridCoord.Y
    );

    return true;
}

/** フロア状態をグリッドとプレイヤーと敵へ反映する */
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

    // フロア定義をGridManagerへ反映
    gridManager->ApplyFloorDefinition(currentFloorDefinition);

    // 先に占有情報をクリア
    gridManager->ClearAllOccupants();

    // 先に敵を初期状態へ戻す
    if (enemyManager)
    {
        enemyManager->ResetAllEnemiesToInitialState();
    }
    else
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ApplyFloorState: EnemyManager未取得のため敵初期化を行いません")
        );
    }

    // プレイヤー開始座標を反映
    playerCharacter->SetCurrentGridCoord(currentFloorDefinition.playerStartGridCoord);
    playerCharacter->SetActorLocation(gridManager->GridToWorld(currentFloorDefinition.playerStartGridCoord));

    // プレイヤー開始位置を占有登録
    if (!gridManager->RegisterOccupant(currentFloorDefinition.playerStartGridCoord, playerCharacter))
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ApplyFloorState: プレイヤー開始位置の占有登録に失敗しました 開始座標=(%d,%d)"),
            currentFloorDefinition.playerStartGridCoord.X,
            currentFloorDefinition.playerStartGridCoord.Y
        );
    }

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("ApplyFloorState: フロア番号=%d 開始座標=(%d,%d) 階段座標=(%d,%d) 壁数=%d"),
        currentFloorIndex,
        currentFloorDefinition.playerStartGridCoord.X,
        currentFloorDefinition.playerStartGridCoord.Y,
        currentFloorDefinition.stairsGridCoord.X,
        currentFloorDefinition.stairsGridCoord.Y,
        currentFloorDefinition.wallCells.Num()
    );

    // フロア開始時はターンを初期状態へ戻す
    if (turnManager)
    {
        turnManager->ResetTurn();
    }
}
