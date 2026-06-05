#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LookAtZombieVerschuerenLain.generated.h"

UCLASS()
class VERSCHUERENLAINZOMBIERUNTIME_API UBTTask_LookAtZombieVerschuerenLain : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_LookAtZombieVerschuerenLain();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetZombieKey;
};
