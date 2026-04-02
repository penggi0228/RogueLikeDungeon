// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Enemies/RldEnemyManager.h"

// Sets default values
ARldEnemyManager::ARldEnemyManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARldEnemyManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARldEnemyManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

