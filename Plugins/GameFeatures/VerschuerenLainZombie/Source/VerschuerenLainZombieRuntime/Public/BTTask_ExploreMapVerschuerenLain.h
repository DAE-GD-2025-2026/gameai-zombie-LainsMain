#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExploreMapVerschuerenLain.generated.h"

UCLASS()
class VERSCHUERENLAINZOMBIERUNTIME_API UBTTask_ExploreMapVerschuerenLain : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ExploreMapVerschuerenLain();

protected:
	// do exploration task
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// key selector for target pos
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	// how far to search for random point
	UPROPERTY(EditAnywhere, Category = "Search")
	float SearchRadius{1500.f};
};
