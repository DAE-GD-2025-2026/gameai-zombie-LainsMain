#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Spin360VerschuerenLain.generated.h"

struct FBTSpin360MemoryVerschuerenLain
{
	float TotalRotated;
	FRotator CurrentRotation;
};

UCLASS()
class VERSCHUERENLAINZOMBIERUNTIME_API UBTTask_Spin360VerschuerenLain : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Spin360VerschuerenLain();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTSpin360MemoryVerschuerenLain); }

	UPROPERTY(EditAnywhere, Category = "Spin")
	float SpinSpeed{360.f};

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector Needs360ScanKey;
};
