#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "BTTask_UseInventoryItemVerschuerenLain.generated.h"

UCLASS()
class VERSCHUERENLAINZOMBIERUNTIME_API UBTTask_UseInventoryItemVerschuerenLain : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseInventoryItemVerschuerenLain();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	EItemType ItemType{EItemType::Food};
};
