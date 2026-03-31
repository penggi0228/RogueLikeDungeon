// RldGameMode.cpp

#include "Game/Core/RldGameMode.h"

#include "Game/Input/RldPlayerController.h"
#include "Game/Characters/RldPlayerCharacter.h"

/** GameModeを初期化する */
ARldGameMode::ARldGameMode()
{
    // プレイヤーコントローラを設定
    PlayerControllerClass = ARldPlayerController::StaticClass();

    // デフォルトPawnを設定
    DefaultPawnClass = ARldPlayerCharacter::StaticClass();
}
