#pragma once

#include "CoreMinimal.h"
#include "CmnInputTypes.generated.h"

/** 共通入力モード（Common層で拡張前提） */
UENUM(BlueprintType)
enum class ECmnInputMode : uint8
{
    Game     UMETA(DisplayName = "Game"),
    Menu     UMETA(DisplayName = "Menu"),
    Dialog   UMETA(DisplayName = "Dialog"),
    Disabled UMETA(DisplayName = "Disabled")
};
