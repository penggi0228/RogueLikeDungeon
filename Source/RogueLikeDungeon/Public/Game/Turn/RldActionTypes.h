// RldActionTypes.h

#pragma once

#include "CoreMinimal.h"
#include "RldActionTypes.generated.h"

class AActor;

/**
 * 行動種別
 * ユニットが実行しようとする行動の種類を表す
 */
UENUM(BlueprintType)
enum class ERldActionType : uint8
{
    /** 行動なし */
    None UMETA(DisplayName = "None"),

    /** 移動 */
    Move UMETA(DisplayName = "Move"),

    /** 待機 */
    Wait UMETA(DisplayName = "Wait"),

    /** 攻撃 */
    Attack UMETA(DisplayName = "Attack"),

    /** 向き変更 */
    Face UMETA(DisplayName = "Face")
};

/**
 * 行動要求
 * 誰がどの行動をどの方向へ行うかをまとめて保持する
 */
USTRUCT(BlueprintType)
struct ROGUELIKEDUNGEON_API FRldActionRequest
{
    GENERATED_BODY()

public:

    // 行動種別
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Turn")
    ERldActionType actionType = ERldActionType::None;

    // 行動方向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Turn")
    FIntPoint direction = FIntPoint::ZeroValue;

    // 行動要求元Actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Turn")
    TObjectPtr<AActor> sourceActor = nullptr;
};
