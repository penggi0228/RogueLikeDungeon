// RldPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Characters/CmnPlayerCharacterBase.h"
#include "RldPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldPlayerCharacter : public ACmnPlayerCharacterBase
{
	GENERATED_BODY()

public:

    ARldPlayerCharacter();

public:

    virtual void RequestMoveDirection(const FIntPoint& Direction) override;
};
