// RldEnemyBase.cpp

#include "Game/Enemies/RldEnemyBase.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

#include "Common/Battle/CmnHealthComponent.h"
#include "Common/Battle/CmnManaComponent.h"
#include "Game/Battle/RldStatusEffectComponent.h"
#include "Game/Characters/RldPlayerCharacter.h"
#include "Game/Grid/RldGridManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldEnemyBase, Log, All);

/** エネミーActorを初期化する */
ARldEnemyBase::ARldEnemyBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // HP管理コンポーネント生成
    healthComponent = CreateDefaultSubobject<UCmnHealthComponent>(TEXT("HealthComponent"));

    // MP管理コンポーネント生成
    manaComponent = CreateDefaultSubobject<UCmnManaComponent>(TEXT("ManaComponent"));

    // 状態異常管理コンポーネント生成
    statusEffectComponent = CreateDefaultSubobject<URldStatusEffectComponent>(TEXT("StatusEffectComponent"));
}

/** 開始時処理 */
void ARldEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    // ステータス定義を読み込んでコンポーネントへ反映
    LoadEnemyStatusDefinition();
    ApplyBattleStatusToComponents();

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
            Verbose,
            TEXT("BeginPlay: Actor=%s 初期位置をグリッドへ補正しました 初期ワールド座標=(%f,%f,%f) 初期グリッド座標=(%d,%d) 補正後ワールド座標=(%f,%f,%f)"),
            *GetNameSafe(this),
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
                TEXT("BeginPlay: Actor=%s 初期位置の占有登録に失敗しました グリッド座標=(%d,%d)"),
                *GetNameSafe(this),
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
            Verbose,
            TEXT("ExecuteTurn: Actor=%s 戦闘不能のため行動しません 現在の座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("ExecuteTurn: Actor=%s GridManager未取得のため行動しません"),
            *GetNameSafe(this)
        );

        return;
    }

    if (!playerCharacter)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ExecuteTurn: Actor=%s PlayerCharacter未取得のため行動しません"),
            *GetNameSafe(this)
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
            TEXT("ExecuteTurn: Actor=%s プレイヤーに隣接しているため攻撃予定です エネミー座標=(%d,%d) プレイヤー座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("ExecuteTurn: Actor=%s 移動先を決定できないため待機します 現在の座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("ExecuteTurn: Actor=%s 次の座標にプレイヤーがいるため移動せず待機します 次の座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("ExecuteTurn: Actor=%s 通行不可のため待機します 次の座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("ExecuteTurn: Actor=%s 占有情報の移動に失敗したため移動を中止します 現在の座標=(%d,%d) 次の座標=(%d,%d)"),
            *GetNameSafe(this),
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
        TEXT("ExecuteTurn: Actor=%s 移動完了 グリッド座標=(%d,%d)"),
        *GetNameSafe(this),
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
        Verbose,
        TEXT("SetInitialGridCoord: Actor=%s 初期座標設定完了 初期座標=(%d,%d)"),
        *GetNameSafe(this),
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
            TEXT("ResetToInitialState: Actor=%s GridManager未取得のため初期状態へ戻せません"),
            *GetNameSafe(this)
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
            TEXT("ResetToInitialState: Actor=%s 初期位置の占有登録に失敗しました 初期座標=(%d,%d)"),
            *GetNameSafe(this),
            initialGridCoord.X,
            initialGridCoord.Y
        );
    }

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("ResetToInitialState: Actor=%s 初期状態へ戻しました 初期座標=(%d,%d)"),
        *GetNameSafe(this),
        initialGridCoord.X,
        initialGridCoord.Y
    );
}

/** エネミーステータス定義を読み込む */
void ARldEnemyBase::LoadEnemyStatusDefinition()
{
    // DataTable未設定時はデフォルトステータスを使用
    if (!enemyStatusDataTable)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("LoadEnemyStatusDefinition: Actor=%s enemyStatusDataTableがnullのためデフォルトステータスを使用します"),
            *GetNameSafe(this)
        );

        return;
    }

    // RowName未設定時はデフォルトステータスを使用
    if (enemyStatusRowName.IsNone())
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("LoadEnemyStatusDefinition: Actor=%s enemyStatusRowNameがNoneのためデフォルトステータスを使用します"),
            *GetNameSafe(this)
        );

        return;
    }

    static const FString contextString = TEXT("RldEnemyStatusDefinitionLookup");

    const FRldEnemyStatusDefinition* foundDefinition =
        enemyStatusDataTable->FindRow<FRldEnemyStatusDefinition>(
            enemyStatusRowName,
            contextString,
            true
        );

    // 対応行未取得時はデフォルトステータスを使用
    if (!foundDefinition)
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("LoadEnemyStatusDefinition: Actor=%s 対応行が存在しないためデフォルトステータスを使用します RowName=%s"),
            *GetNameSafe(this),
            *enemyStatusRowName.ToString()
        );

        return;
    }

    currentBattleStatus = foundDefinition->battleStatus;

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("LoadEnemyStatusDefinition: データロード完了 Actor=%s RowName=%s エネミーID=%s 表示名=%s 最大HP=%d 最大MP=%d 攻撃力=%d 防御力=%d"),
        *GetNameSafe(this),
        *enemyStatusRowName.ToString(),
        *foundDefinition->enemyId.ToString(),
        *foundDefinition->displayName.ToString(),
        currentBattleStatus.maxHP,
        currentBattleStatus.maxMP,
        currentBattleStatus.attackPower,
        currentBattleStatus.defensePower
    );
}

/** 戦闘ステータスをコンポーネントへ反映する */
void ARldEnemyBase::ApplyBattleStatusToComponents()
{
    // HP管理コンポーネント取得時は最大HPの値を更新して、現在のHPに反映する
    if (healthComponent)
    {
        healthComponent->SetMaxHP(currentBattleStatus.maxHP);
        healthComponent->ResetHP();
    }
    else
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ApplyBattleStatusToComponents: Actor=%s HealthComponentがnullのため最大HPを反映できません"),
            *GetNameSafe(this)
        );
    }

    // MP管理コンポーネント取得時は最大MPの値を更新して、現在のMPに反映する
    if (manaComponent)
    {
        manaComponent->SetMaxMP(currentBattleStatus.maxMP);
        manaComponent->ResetMP();
    }
    else
    {
        UE_LOG(
            LogRldEnemyBase,
            Warning,
            TEXT("ApplyBattleStatusToComponents: Actor=%s ManaComponentがnullのため最大MPを反映できません"),
            *GetNameSafe(this)
        );
    }

    UE_LOG(
        LogRldEnemyBase,
        Log,
        TEXT("ApplyBattleStatusToComponents: Actor=%s 戦闘ステータス反映完了 最大HP=%d 最大MP=%d 攻撃力=%d 防御力=%d"),
        *GetNameSafe(this),
        currentBattleStatus.maxHP,
        currentBattleStatus.maxMP,
        currentBattleStatus.attackPower,
        currentBattleStatus.defensePower
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
            TEXT("ResolveGridManager: Actor=%s ARldGridManagerがレベル上に見つからないためグリッド移動処理が制限されます"),
            *GetNameSafe(this)
        );

        return;
    }

    UE_LOG(
        LogRldEnemyBase,
        Verbose,
        TEXT("ResolveGridManager: Actor=%s GridManager=%s クラス=%s"),
        *GetNameSafe(this),
        *GetNameSafe(gridManager),
        *GetNameSafe(gridManager->GetClass())
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
            TEXT("ResolvePlayerCharacter: Actor=%s ARldPlayerCharacterがレベル上に見つからないためプレイヤー追跡処理が制限されます"),
            *GetNameSafe(this)
        );

        return;
    }

    UE_LOG(
        LogRldEnemyBase,
        Verbose,
        TEXT("ResolvePlayerCharacter: Actor=%s PlayerCharacter=%s クラス=%s"),
        *GetNameSafe(this),
        *GetNameSafe(playerCharacter),
        *GetNameSafe(playerCharacter->GetClass())
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
