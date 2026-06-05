#include "BTTask_AttackZombieVerschuerenLain.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_AttackZombieVerschuerenLain::UBTTask_AttackZombieVerschuerenLain()
{
	NodeName = TEXT("Attack Zombie VerschuerenLain");
}

EBTNodeResult::Type UBTTask_AttackZombieVerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!Controller || !BlackboardComp) return EBTNodeResult::Failed;

	ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
	if (!Survivor) return EBTNodeResult::Failed;

	AActor* ZombieActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetZombieKey.SelectedKeyName));
	if (!ZombieActor) return EBTNodeResult::Failed;

	UInventoryComponent* InvComp = Survivor->GetComponentByClass<UInventoryComponent>();
	if (!InvComp) return EBTNodeResult::Failed;

	// find weapon in inventory
	int WeaponSlot = -1;
	const TArray<ABaseItem*>& Inv = InvComp->GetInventory();
	for (int s = 0; s < Inv.Num(); ++s)
	{
		if (Inv[s])
		{
			EItemType Type = Inv[s]->GetItemType();
			if ((Type == EItemType::Pistol || Type == EItemType::Shotgun) && Inv[s]->GetValue() > 0)
			{
				WeaponSlot = s;
				break;
			}
		}
	}

	if (WeaponSlot != -1)
	{
		// rotate to face zombie
		FVector Dir = (ZombieActor->GetActorLocation() - Survivor->GetActorLocation()).GetSafeNormal2D();
		if (!Dir.IsNearlyZero())
		{
			FRotator Rot = Dir.Rotation();
			Survivor->SetActorRotation(Rot);
			Controller->SetControlRotation(Rot);
		}

		// shoot
		if (InvComp->UseItem(WeaponSlot))
		{
			// remove weapon if out of ammo
			if (Inv[WeaponSlot]->GetValue() <= 0)
			{
				InvComp->RemoveItem(WeaponSlot);
			}
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
