#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FleeZombieVerschuerenLain.generated.h"

UCLASS()
class VERSCHUERENLAINZOMBIERUNTIME_API UBTTask_FleeZombieVerschuerenLain : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FleeZombieVerschuerenLain();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetZombieKey;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeDistance{1500.f};
};
