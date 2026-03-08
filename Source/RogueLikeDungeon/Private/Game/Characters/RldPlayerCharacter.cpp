// RldPlayerCharacter.cpp

#include "Game/Characters/RldPlayerCharacter.h"

void ARldPlayerCharacter::RequestMoveDirection(const FIntPoint& Direction)
{
    FVector Offset = FVector(Direction.X * 100.f, Direction.Y * 100.f, 0.f);

    SetActorLocation(GetActorLocation() + Offset);
}

