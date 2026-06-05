#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MarkHouseExploredVerschuerenLain.generated.h"

UCLASS()
class VERSCHUERENLAINZOMBIERUNTIME_API UBTTask_MarkHouseExploredVerschuerenLain : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MarkHouseExploredVerschuerenLain();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
