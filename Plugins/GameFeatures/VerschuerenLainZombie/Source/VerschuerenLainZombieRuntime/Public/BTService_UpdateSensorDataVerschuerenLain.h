#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateSensorDataVerschuerenLain.generated.h"

UCLASS()
class VERSCHUERENLAINZOMBIERUNTIME_API UBTService_UpdateSensorDataVerschuerenLain : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateSensorDataVerschuerenLain();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetZombieKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasTargetZombieKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HealthKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StaminaKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasWeaponKey;
};
