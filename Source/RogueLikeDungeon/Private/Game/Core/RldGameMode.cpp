// RldGameMode.cpp

#include "Game/Core/RldGameMode.h"

#include "Game/Input/RldPlayerController.h"
#include "Game/Characters/RldPlayerCharacter.h"

ARldGameMode::ARldGameMode()
{
    // プレイヤーコントローラをC++で固定
    PlayerControllerClass = ARldPlayerController::StaticClass();

    // デフォルトPawnをC++で固定
     DefaultPawnClass = ARldPlayerCharacter::StaticClass();
}
