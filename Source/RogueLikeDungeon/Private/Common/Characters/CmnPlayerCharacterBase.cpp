// CmnPlayerCharacterBase.cpp

#include "Common/Characters/CmnPlayerCharacterBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnPlayerCharacterBase, Log, All);

ACmnPlayerCharacterBase::ACmnPlayerCharacterBase()
{
    // 共通基底ではTick不要
    PrimaryActorTick.bCanEverTick = false;
}

/**
 * 移動方向入力リクエストを受け取る
 */
void ACmnPlayerCharacterBase::RequestMoveDirection(const FIntPoint& Direction)
{
    UE_LOG(
        LogCmnPlayerCharacterBase,
        Verbose,
        TEXT("RequestMoveDirection: X=%d Y=%d"),
        Direction.X,
        Direction.Y
    );
}

/**
 * 向き変更入力リクエストを受け取る
 */
void ACmnPlayerCharacterBase::RequestFaceDirection(const FIntPoint& Direction)
{
    UE_LOG(
        LogCmnPlayerCharacterBase,
        Verbose,
        TEXT("RequestFaceDirection: X=%d Y=%d"),
        Direction.X,
        Direction.Y
    );
}

/**
 * カメラ視点入力リクエストを受け取る
 */
void ACmnPlayerCharacterBase::RequestLookInput(const FVector2D& Axis)
{
    UE_LOG(
        LogCmnPlayerCharacterBase,
        Verbose,
        TEXT("RequestLookInput: X=%f Y=%f"),
        Axis.X,
        Axis.Y
    );
}

/**
 * カメラズーム入力リクエストを受け取る
 */
void ACmnPlayerCharacterBase::RequestZoomInput(float Value)
{
    UE_LOG(
        LogCmnPlayerCharacterBase,
        Verbose,
        TEXT("RequestZoomInput: Value=%f"),
        Value
    );
}

/**
 * 移動入力を受け付け可能か判定する
 */
bool ACmnPlayerCharacterBase::CanAcceptMoveInput() const
{
    return true;
}

/**
 * 向き変更入力を受け付け可能か判定する
 */
bool ACmnPlayerCharacterBase::CanAcceptFaceInput() const
{
    return true;
}

/**
 * 視点入力を受け付け可能か判定する
 */
bool ACmnPlayerCharacterBase::CanAcceptLookInput() const
{
    return true;
}

/**
 * ズーム入力を受け付け可能か判定する
 */
bool ACmnPlayerCharacterBase::CanAcceptZoomInput() const
{
    return true;
}
