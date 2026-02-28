// RldGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RldGameMode.generated.h"

/** ゲーム全体のデフォルトクラスを決めるGameMode */
UCLASS()
class ROGUELIKEDUNGEON_API ARldGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ARldGameMode();
};