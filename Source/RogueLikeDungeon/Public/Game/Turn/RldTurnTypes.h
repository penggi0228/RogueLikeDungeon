// RldTurnTypes.h

#pragma once

#include "CoreMinimal.h"
#include "RldTurnTypes.generated.h"

/**
 * ターン進行フェーズ
 * ターン中に現在どの処理段階かを表す
 */
UENUM(BlueprintType)
enum class ERldTurnPhase : uint8
{
    /** プレイヤー入力受付中 */
    PlayerInput UMETA(DisplayName = "PlayerInput"),

    /** プレイヤー行動解決中 */
    PlayerAction UMETA(DisplayName = "PlayerAction"),

    /** 敵行動解決中 */
    EnemyAction UMETA(DisplayName = "EnemyAction"),

    /** ターン終了処理中 */
    TurnEnd UMETA(DisplayName = "TurnEnd")
};
