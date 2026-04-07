// CmnTurnActorInterface.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CmnTurnActorInterface.generated.h"

/**
 * ターン行動用インターフェース
 * ターンで行動するActorの共通入口を定義する
 */
UINTERFACE(Blueprintable)
class ROGUELIKEDUNGEON_API UCmnTurnActorInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * ターン行動用インターフェース本体
 */
class ROGUELIKEDUNGEON_API ICmnTurnActorInterface
{
    GENERATED_BODY()

public:

    /**
     * 1ターン分の行動を実行する
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Cmn|Turn")
    void ExecuteTurn();
};
