// RldGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RldGameMode.generated.h"

/** ゲーム全体のデフォルトクラスを設定するGameMode */
UCLASS()
class ROGUELIKEDUNGEON_API ARldGameMode : public AGameMode
{
    GENERATED_BODY()

public:

    /** GameModeを初期化する */
    ARldGameMode();
};
