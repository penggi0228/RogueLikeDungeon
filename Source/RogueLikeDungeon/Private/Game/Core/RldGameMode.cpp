// RldGameMode.cpp

#include "Game/Core/RldGameMode.h"

#include "Game/Input/RldPlayerController.h"
// Pawnも用意したら include して DefaultPawnClass を指定する

ARldGameMode::ARldGameMode()
{
    // プレイヤーコントローラをC++で固定
    PlayerControllerClass = ARldPlayerController::StaticClass();

    // Pawnはまだ作ってないなら触らなくてOK
    // DefaultPawnClass = ARldYourPawn::StaticClass();
}