// RldFloorManager.cpp

#include "Game/Floor/RldFloorManager.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

#include "Common/Debug/CmnDebugCategories.h"
#include "Common/Debug/CmnDebugWorldSubsystem.h"
#include "Game/Characters/RldPlayerCharacter.h"
#include "Game/Enemies/RldEnemyManager.h"
#include "Game/Grid/RldGridManager.h"
#include "Game/Turn/RldTurnManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldFloorManager, Log, All);

/** フロア管理Actorを初期化する */
ARldFloorManager::ARldFloorManager()
{
    PrimaryActorTick.bCanEverTick = true;

    sectionBoundsDebugStyle.drawColor = FColor::Cyan;
    sectionBoundsDebugStyle.bPersistentLines = false;
    sectionBoundsDebugStyle.duration = 0.15f;
    sectionBoundsDebugStyle.lineThickness = 2.0f;
    sectionBoundsDebugStyle.zOffset = 60.0f;
    sectionBoundsDebugStyle.sizeScale = 0.25f;

    playerStartDebugStyle.drawColor = FColor::Yellow;
    playerStartDebugStyle.bPersistentLines = false;
    playerStartDebugStyle.duration = 0.15f;
    playerStartDebugStyle.lineThickness = 2.0f;
    playerStartDebugStyle.zOffset = 80.0f;
    playerStartDebugStyle.sizeScale = 0.35f;

    stairsHighlightDebugStyle.drawColor = FColor::Magenta;
    stairsHighlightDebugStyle.bPersistentLines = false;
    stairsHighlightDebugStyle.duration = 0.15f;
    stairsHighlightDebugStyle.lineThickness = 2.0f;
    stairsHighlightDebugStyle.zOffset = 100.0f;
    stairsHighlightDebugStyle.sizeScale = 0.40f;
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

/** 毎フレーム処理 */
void ARldFloorManager::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);

    UpdateContinuousDebugDraw(deltaSeconds);
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
    FCmnGridLayoutBuildResult builtFloorLayout;

    // フロア定義読込失敗時は開始しない
    if (!TryLoadFloorDefinition(targetFloorIndex, loadedFloorDefinition))
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("StartFloorAt: フロア定義読み込みに失敗したため開始しません フロア番号=%d"),
            targetFloorIndex
        );

        return;
    }

    currentFloorIndex = targetFloorIndex;
    currentFloorDefinition = loadedFloorDefinition;

    // フロア生成失敗時は開始しない
    if (!TryBuildFloorLayout(builtFloorLayout))
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("StartFloorAt: フロア生成に失敗したため開始しません フロア番号=%d"),
            currentFloorIndex
        );

        return;
    }

    currentFloorLayout = builtFloorLayout;

    // 常時デバッグ描画の状態を初期化
    continuousDebugDrawElapsed = 0.0f;
    bDebugInfoLogged = false;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("StartFloorAt: フロア準備完了 フロア番号=%d グリッドサイズ=(%d,%d) 床数=%d 壁数=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        currentFloorIndex,
        currentFloorLayout.gridWidth,
        currentFloorLayout.gridHeight,
        currentFloorLayout.floorCells.Num(),
        currentFloorLayout.wallCells.Num(),
        currentFloorLayout.playerStartGridCoord.X,
        currentFloorLayout.playerStartGridCoord.Y,
        currentFloorLayout.stairsGridCoord.X,
        currentFloorLayout.stairsGridCoord.Y
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
        TEXT("GoToNextFloor: 次フロアへ進みます 現在のフロア番号=%d 次フロア番号=%d"),
        currentFloorIndex,
        nextFloorIndex
    );

    StartFloorAt(nextFloorIndex);
}

/** 現在のフロアのデバッグ描画を行う */
void ARldFloorManager::DrawDebugFloorState() const
{
    DrawDebugFloorStateInternal(true);
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
            TEXT("ResolveGridManager: ARldGridManagerがレベル上に見つからないためフロア状態を反映できません")
        );

        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Verbose,
        TEXT("ResolveGridManager: Actor=%s クラス=%s"),
        *GetNameSafe(gridManager),
        *GetNameSafe(gridManager->GetClass())
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
            TEXT("ResolvePlayerCharacter: ARldPlayerCharacterがレベル上に見つからないためプレイヤー開始位置を反映できません")
        );

        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Verbose,
        TEXT("ResolvePlayerCharacter: Actor=%s クラス=%s"),
        *GetNameSafe(playerCharacter),
        *GetNameSafe(playerCharacter->GetClass())
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
            TEXT("ResolveTurnManager: ARldTurnManagerがレベル上に見つからないためターン初期化を行いません")
        );

        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Verbose,
        TEXT("ResolveTurnManager: Actor=%s クラス=%s 現在のターン数=%d"),
        *GetNameSafe(turnManager),
        *GetNameSafe(turnManager->GetClass()),
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
            TEXT("ResolveEnemyManager: ARldEnemyManagerがレベル上に見つからないため敵配置処理を行いません")
        );

        return;
    }

    UE_LOG(
        LogRldFloorManager,
        Verbose,
        TEXT("ResolveEnemyManager: Actor=%s クラス=%s エネミー数=%d"),
        *GetNameSafe(enemyManager),
        *GetNameSafe(enemyManager->GetClass()),
        enemyManager->GetEnemyCount()
    );
}

/** 指定フロア番号に対応するRowNameを作成する */
FName ARldFloorManager::BuildFloorRowName(int32 floorIndex) const
{
    return FName(*FString::Printf(TEXT("Floor_%03d"), floorIndex));
}

/** 指定フロア番号の定義を読込する */
bool ARldFloorManager::TryLoadFloorDefinition(int32 floorIndex, FRldFloorDefinition& outFloorDefinition) const
{
    // DataTable未設定時は読込失敗
    if (!floorDefinitionDataTable)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("TryLoadFloorDefinition: floorDefinitionDataTableがnullのため読み込みに失敗しました フロア番号=%d"),
            floorIndex
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
            TEXT("TryLoadFloorDefinition: 対応行が存在しないため読み込みに失敗しました フロア番号=%d RowName=%s"),
            floorIndex,
            *rowName.ToString()
        );

        return false;
    }

    outFloorDefinition = *foundDefinition;

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("TryLoadFloorDefinition: データロード完了 フロア番号=%d RowName=%s グリッドサイズ=(%d,%d) 自動生成レイアウト=%s"),
        floorIndex,
        *rowName.ToString(),
        outFloorDefinition.gridWidth,
        outFloorDefinition.gridHeight,
        outFloorDefinition.bUseProceduralLayout ? TEXT("有効") : TEXT("無効")
    );

    return true;
}

/** 現在のフロア定義からレイアウトを生成する */
bool ARldFloorManager::TryBuildFloorLayout(FCmnGridLayoutBuildResult& outFloorLayout)
{
    const bool bSucceeded = floorGenerator.GenerateFloorLayout(
        currentFloorDefinition,
        outFloorLayout
    );

    if (!bSucceeded)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("TryBuildFloorLayout: レイアウト生成に失敗しました フロア番号=%d"),
            currentFloorIndex
        );

        return false;
    }

    return true;
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
            TEXT("ApplyFloorState: GridManager未取得のため反映できません フロア番号=%d"),
            currentFloorIndex
        );

        return;
    }

    // PlayerCharacter未取得時は反映しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ApplyFloorState: PlayerCharacter未取得のため反映できません フロア番号=%d"),
            currentFloorIndex
        );

        return;
    }

    // フロア状態をGridManagerへ反映
    gridManager->ApplyFloorLayout(currentFloorDefinition, currentFloorLayout);

    if (enemyManager)
    {
        // 固定フロアから遷移した場合に備えて自動生成敵を先に破棄
        enemyManager->DestroyAllRuntimeSpawnedEnemies();

        if (currentFloorDefinition.bUseProceduralLayout)
        {
            enemyManager->SpawnEnemiesForProceduralFloor(
                currentFloorDefinition,
                currentFloorLayout,
                gridManager
            );
        }
        else
        {
            enemyManager->ResetAllEnemiesToInitialState();
        }
    }
    else
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ApplyFloorState: EnemyManager未取得のため敵処理を行いません フロア番号=%d"),
            currentFloorIndex
        );
    }

    // プレイヤー開始座標を反映
    playerCharacter->SetCurrentGridCoord(currentFloorLayout.playerStartGridCoord);
    playerCharacter->SetActorLocation(gridManager->GridToWorld(currentFloorLayout.playerStartGridCoord));

    // プレイヤー開始位置を占有登録
    if (!gridManager->RegisterOccupant(currentFloorLayout.playerStartGridCoord, playerCharacter))
    {
        UE_LOG(
            LogRldFloorManager,
            Warning,
            TEXT("ApplyFloorState: プレイヤー開始位置の占有登録に失敗しました Actor=%s 開始座標=(%d,%d)"),
            *GetNameSafe(playerCharacter),
            currentFloorLayout.playerStartGridCoord.X,
            currentFloorLayout.playerStartGridCoord.Y
        );
    }

    UE_LOG(
        LogRldFloorManager,
        Log,
        TEXT("ApplyFloorState: フロア反映完了 フロア番号=%d 開始座標=(%d,%d) 階段座標=(%d,%d) 床数=%d 壁数=%d"),
        currentFloorIndex,
        currentFloorLayout.playerStartGridCoord.X,
        currentFloorLayout.playerStartGridCoord.Y,
        currentFloorLayout.stairsGridCoord.X,
        currentFloorLayout.stairsGridCoord.Y,
        currentFloorLayout.floorCells.Num(),
        currentFloorLayout.wallCells.Num()
    );

    // フロア開始時はターンを初期状態へ戻す
    if (turnManager)
    {
        turnManager->ResetTurn();
    }

    // フロア反映直後の手動描画としてログありで描画する
    if (bDrawDebugOnApplyFloorState)
    {
        DrawDebugFloorState();
        LogDebugDrawInfo();
    }
}

/** フロアデバッグ描画情報をログ出力する */
void ARldFloorManager::LogDebugDrawInfo()
{
    if (bDebugInfoLogged)
    {
        return;
    }

    bDebugInfoLogged = true;

    UE_LOG(
        LogRldFloorManager,
        Verbose,
        TEXT("LogDebugDrawInfo: フロアデバッグ描画凡例 緑=床マス 赤=壁マス 青=階段マス 水色=セクション外枠 黄=プレイヤー開始位置 紫=階段強調")
    );

    UE_LOG(
        LogRldFloorManager,
        Verbose,
        TEXT("LogDebugDrawInfo: フロアデバッグ描画情報 フロア番号=%d セクション数=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        currentFloorIndex,
        currentFloorLayout.sections.Num(),
        currentFloorLayout.playerStartGridCoord.X,
        currentFloorLayout.playerStartGridCoord.Y,
        currentFloorLayout.stairsGridCoord.X,
        currentFloorLayout.stairsGridCoord.Y
    );
}

/** フロアデバッグ描画を更新する */
void ARldFloorManager::UpdateContinuousDebugDraw(float deltaSeconds)
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
    DrawDebugFloorStateInternal(false);
}

/** フロア常時デバッグ描画が有効か判定する */
bool ARldFloorManager::ShouldDrawContinuousDebug() const
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
        && debugSubsystem->IsCategoryEnabled(CmnDebugCategories::Floor);
}

/** 現在のフロアのデバッグ描画を行う */
void ARldFloorManager::DrawDebugFloorStateInternal(bool bOutputLog) const
{
    UWorld* world = GetWorld();

    // World未取得時は描画しない
    if (!world)
    {
        if (bOutputLog)
        {
            UE_LOG(
                LogRldFloorManager,
                Warning,
                TEXT("DrawDebugFloorState: World未取得のため描画しません")
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
                LogRldFloorManager,
                Warning,
                TEXT("DrawDebugFloorState: DebugSubsystem未取得のため描画しません")
            );
        }

        return;
    }

    // デバッグ描画全体またはFloorカテゴリが無効な場合は描画しない
    if (!debugSubsystem->IsDebugEnabled() || !debugSubsystem->IsCategoryEnabled(CmnDebugCategories::Floor))
    {
        return;
    }

    // 手動描画時のみGridManager側の床・壁・階段描画を実行
    // 常時描画ではGridManager側のログあり描画を呼ばない
    if (bOutputLog)
    {
        if (gridManager)
        {
            gridManager->DrawDebugGridState();
        }
        else
        {
            UE_LOG(
                LogRldFloorManager,
                Warning,
                TEXT("DrawDebugFloorState: GridManager未取得のためグリッド描画を行いません")
            );
        }
    }

    // セクション外枠を描画
    if (bDrawSectionBounds)
    {
        for (const FCmnGridSection& section : currentFloorLayout.sections)
        {
            debugSubsystem->DrawGridSectionBounds(
                currentFloorDefinition.gridDefinition,
                section,
                sectionBoundsDebugStyle
            );
        }
    }

    // 開始位置を描画
    if (bDrawPlayerStartCell)
    {
        debugSubsystem->DrawGridCell(
            currentFloorDefinition.gridDefinition,
            currentFloorLayout.playerStartGridCoord,
            playerStartDebugStyle
        );
    }

    // 階段位置を強調描画
    if (bDrawStairsHighlightCell)
    {
        debugSubsystem->DrawGridCell(
            currentFloorDefinition.gridDefinition,
            currentFloorLayout.stairsGridCoord,
            stairsHighlightDebugStyle
        );
    }

    if (bOutputLog)
    {
        UE_LOG(
            LogRldFloorManager,
            Log,
            TEXT("DrawDebugFloorState: フロアデバッグ描画完了 フロア番号=%d セクション数=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
            currentFloorIndex,
            currentFloorLayout.sections.Num(),
            currentFloorLayout.playerStartGridCoord.X,
            currentFloorLayout.playerStartGridCoord.Y,
            currentFloorLayout.stairsGridCoord.X,
            currentFloorLayout.stairsGridCoord.Y
        );
    }
}
