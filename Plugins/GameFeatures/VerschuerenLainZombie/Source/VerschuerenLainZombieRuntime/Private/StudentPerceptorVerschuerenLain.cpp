// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptorVerschuerenLain.h"


UStudentPerceptorVerschuerenLain::UStudentPerceptorVerschuerenLain()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptorVerschuerenLain::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptorVerschuerenLain::OnPerceptionUpdated);
	}
}

void UStudentPerceptorVerschuerenLain::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, 
		FString::Printf(TEXT("Saw Something!")));
}
