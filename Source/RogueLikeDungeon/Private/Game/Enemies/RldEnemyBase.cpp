// RldEnemyBase.cpp

#include "Game/Enemies/RldEnemyBase.h"

#include "Kismet/GameplayStatics.h"

#include "Common/Battle/CmnHealthComponent.h"
#include "Game/Characters/RldPlayerCharacter.h"
#include "Game/Grid/RldGridManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldEnemyBase, Log, All);

/** エネミーActorを初期化する */
ARldEnemyBase::ARldEnemyBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // HP管理Component生成
    healthComponent = CreateDefaultSubobject<UCmnHealthComponent>(TEXT("HealthComponent"));
}

/** 開始時処理 */
void ARldEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    // 各管理Actorを取得
    ResolveGridManager();
    ResolvePlayerCharacter();

    // 現在位置から初期グリッド座標を取得
    if (gridManager)
    {
        const FVector currentWorldLocation = GetActorLocation();
        const FIntPoint initialSpawnGridCoord = gridManager->WorldToGrid(currentWorldLocation);

        SetCurrentGridCoord(initialSpawnGridCoord);
        SetInitialGridCoord(initialSpawnGridCoord);

        // グリッド中心位置へ補正
        const FVector snappedWorldLocation = gridManager->GridToWorld(initialSpawnGridCoord);
        SetActorLocation(snappedWorldLocation);

        UE_LOG(
            LogRldEnemyBase,
            Log,
            TEXT("BeginPlay: 初期ワールド座標=(%f,%f,%f) 初期グリッド座標=(%d,%d) 補正後ワールド座標=(%f,%f,%f)"),
            currentWorldLocation.X,
            currentWorldLocation.Y,
            currentWorldLocation.Z,
            initialSpawnGridCoord.X,
            initialSpawnGridCoord.Y,
            snappedWorldLocation.X,
            snappedWorldLocation.Y,
            snappedWorldLocation.Z
        );

        // 初期位置を占有登録
        if (!gridManager->RegisterOccupant(initialSpawnGridCoord, this))
        {
            UE_LOG(
                LogRldEnemyBase,
                Warning,
                TEXT("BeginPlay: 初期位置の占有登録に失敗しました グリッド座標=(%d,%d)"),
                initialSpawnGridCoord.X,
                initialSpawnGridCoord.Y
            );
        }
    }
}

/** 1ターン分の行動を実行する */
void ARldEnemyBase::ExecuteTurn_Implementation()
{
    // 戦闘不能時は行動しない
    if (healthComponent && healthComponent->IsDead())
    {
        UE_LOG(
            LogRldEnemyBase,
            Log,
            TEXT("ExecuteTurn: 戦闘不能のため行動しません 現在座標=(%d,%d)"),
            GetCurrentGridCoord().X,
            GetCurrentGridCoord().Y
        );
        return;
    }

    // 必要参照未取得時は行動しない
    if (!gridManager)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ExecuteTurn: GridManager未取得のため行動しません")
        );
        return;
    }

    if (!playerCharacter)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ExecuteTurn: PlayerCharacter未取得のため行動しません")
        );
        return;
    }

    const FIntPoint currentEnemyGridCoord = GetCurrentGridCoord();
    const FIntPoint playerGridCoord = playerCharacter->GetCurrentGridCoord();

    const int32 distanceX = FMath::Abs(playerGridCoord.X - currentEnemyGridCoord.X);
    const int32 distanceY = FMath::Abs(playerGridCoord.Y - currentEnemyGridCoord.Y);

    // プレイヤーに隣接している場合は攻撃予定ログだけ出す
    if ((distanceX + distanceY) == 1)
    {
        UE_LOG(
            LogRldEnemyBase,
            Log,
            TEXT("ExecuteTurn: プレイヤーに隣接しているため攻撃予定です エネミー座標=(%d,%d) プレイヤー座標=(%d,%d)"),
            currentEnemyGridCoord.X,
            currentEnemyGridCoord.Y,
            playerGridCoord.X,
            playerGridCoord.Y
        );
        return;
    }

    FIntPoint nextGridCoord;

    // 次移動先を決められない場合は待機
    if (!TryBuildNextMoveTarget(nextGridCoord))
    {
        UE_LOG(
            LogRldEnemyBase,
            Log,
            TEXT("ExecuteTurn: 移動先を決定できないため待機します 現在座標=(%d,%d)"),
            currentEnemyGridCoord.X,
            currentEnemyGridCoord.Y
        );
        return;
    }

    // プレイヤーのいるマスへは進入しない
    if (nextGridCoord == playerGridCoord)
    {
        UE_LOG(
            LogRldEnemyBase,
            Log,
            TEXT("ExecuteTurn: 次座標がプレイヤー位置のため移動せず待機します 次座標=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    // 通行不可なら待機
    if (!gridManager->IsWalkable(nextGridCoord))
    {
        UE_LOG(
            LogRldEnemyBase,
            Log,
            TEXT("ExecuteTurn: 通行不可のため待機します 次座標=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    // 占有情報を移動
    if (!gridManager->MoveOccupant(currentEnemyGridCoord, nextGridCoord, this))
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ExecuteTurn: 占有情報の移動に失敗したため移動を中止します 現在座標=(%d,%d) 次座標=(%d,%d)"),
            currentEnemyGridCoord.X,
            currentEnemyGridCoord.Y,
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    SetCurrentGridCoord(nextGridCoord);
    SetActorLocation(gridManager->GridToWorld(nextGridCoord));

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("ExecuteTurn: エネミーが移動しました 次座標=(%d,%d)"),
        nextGridCoord.X,
        nextGridCoord.Y
    );
}

/** 初期グリッド座標を設定する */
void ARldEnemyBase::SetInitialGridCoord(const FIntPoint& newInitialGridCoord)
{
    initialGridCoord = newInitialGridCoord;

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("SetInitialGridCoord: 初期座標=(%d,%d)"),
        initialGridCoord.X,
        initialGridCoord.Y
    );
}

/** 初期状態へ戻す */
void ARldEnemyBase::ResetToInitialState()
{
    // GridManager未取得時は位置を戻せない
    if (!gridManager)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ResetToInitialState: GridManager未取得のため初期状態へ戻せません")
        );
        return;
    }

    const FIntPoint previousGridCoord = GetCurrentGridCoord();

    // 現在位置に自分が登録されている場合のみ解除
    if (gridManager->GetOccupyingActor(previousGridCoord) == this)
    {
        gridManager->UnregisterOccupant(previousGridCoord, this);
    }

    SetCurrentGridCoord(initialGridCoord);
    SetActorLocation(gridManager->GridToWorld(initialGridCoord));

    if (!gridManager->RegisterOccupant(initialGridCoord, this))
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ResetToInitialState: 初期位置の占有登録に失敗しました 初期座標=(%d,%d)"),
            initialGridCoord.X,
            initialGridCoord.Y
        );
    }

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("ResetToInitialState: 初期状態へ戻しました 初期座標=(%d,%d)"),
        initialGridCoord.X,
        initialGridCoord.Y
    );
}

/** グリッド管理Actorを取得する */
void ARldEnemyBase::ResolveGridManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldGridManager::StaticClass());
    gridManager = Cast<ARldGridManager>(foundActor);

    if (!gridManager)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ResolveGridManager: ARldGridManagerがレベル上に見つからない")
        );
        return;
    }

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("ResolveGridManager: 名前=%s クラス=%s"),
        *gridManager->GetName(),
        *gridManager->GetClass()->GetName()
    );
}

/** プレイヤーキャラクターを取得する */
void ARldEnemyBase::ResolvePlayerCharacter()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldPlayerCharacter::StaticClass());
    playerCharacter = Cast<ARldPlayerCharacter>(foundActor);

    if (!playerCharacter)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ResolvePlayerCharacter: ARldPlayerCharacterがレベル上に見つからない")
        );
        return;
    }

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("ResolvePlayerCharacter: 名前=%s クラス=%s"),
        *playerCharacter->GetName(),
        *playerCharacter->GetClass()->GetName()
    );
}

/** プレイヤー方向への移動候補を求める */
bool ARldEnemyBase::TryBuildNextMoveTarget(FIntPoint& outTargetGridCoord) const
{
    outTargetGridCoord = GetCurrentGridCoord();

    if (!playerCharacter)
    {
        return false;
    }

    const FIntPoint playerGridCoord = playerCharacter->GetCurrentGridCoord();
    const FIntPoint currentEnemyGridCoord = GetCurrentGridCoord();
    const FIntPoint delta = playerGridCoord - currentEnemyGridCoord;

    const int32 absX = FMath::Abs(delta.X);
    const int32 absY = FMath::Abs(delta.Y);

    // 同位置なら移動不要
    if (absX == 0 && absY == 0)
    {
        return false;
    }

    // 主軸優先で1マス分だけ移動候補を決める
    if (absX >= absY)
    {
        outTargetGridCoord.X += (delta.X > 0) ? 1 : -1;
    }
    else
    {
        outTargetGridCoord.Y += (delta.Y > 0) ? 1 : -1;
    }

    return true;
}
