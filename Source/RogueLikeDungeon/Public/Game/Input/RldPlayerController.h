#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Common/Input/CmnInputTypes.h"
#include "RldPlayerController.generated.h"

class UEnhancedInputComponent;
class UCmnInputRouter;

/**
 * ゲーム固有PlayerController
 * - Enhanced InputのBindを担当
 * - 入力解釈は CmnInputRouter に委譲
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void PlayerTick(float DeltaTime) override;

    /** 入力モードを切替（Game / Menu / Dialog / Disabled） */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input")
    void SetCommonInputMode(ECmnInputMode Mode);

    UCmnInputRouter* GetInputRouter() const { return InputRouter; }

private:
    void OnMoveStarted(const FInputActionValue& Value);
    void OnUINavigateTriggered(const FInputActionValue& Value);

private:
    UPROPERTY()
    TObjectPtr<UCmnInputRouter> InputRouter;
};
