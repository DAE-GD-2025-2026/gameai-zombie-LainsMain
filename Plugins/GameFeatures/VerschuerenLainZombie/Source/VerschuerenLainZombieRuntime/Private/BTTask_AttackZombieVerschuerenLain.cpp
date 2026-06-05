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

	// determine best weapon based on distance
	float DistToZombie = FVector::Dist(Survivor->GetActorLocation(), ZombieActor->GetActorLocation());

	int ShotgunSlot = -1;
	int PistolSlot = -1;

	const TArray<ABaseItem*>& Inv = InvComp->GetInventory();
	for (int s = 0; s < Inv.Num(); ++s)
	{
		if (Inv[s] && Inv[s]->GetValue() > 0)
		{
			EItemType Type = Inv[s]->GetItemType();
			if (Type == EItemType::Shotgun)
			{
				ShotgunSlot = s;
			}
			else if (Type == EItemType::Pistol)
			{
				PistolSlot = s;
			}
		}
	}

	int WeaponSlot = -1;
	if (DistToZombie <= 600.f)
	{
		WeaponSlot = (ShotgunSlot != -1) ? ShotgunSlot : PistolSlot;
	}
	else
	{
		WeaponSlot = (PistolSlot != -1) ? PistolSlot : ShotgunSlot;
	}

	if (WeaponSlot != -1)
	{
		// smooth rotate to face zombie using AI focus
		Controller->SetFocus(ZombieActor);

		// Line of sight check to save ammo
		FHitResult HitResult;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(Survivor);
		TraceParams.AddIgnoredActor(ZombieActor);

		FVector StartTrace = Survivor->GetActorLocation() + FVector(0.f, 0.f, 50.f);
		FVector EndTrace = ZombieActor->GetActorLocation() + FVector(0.f, 0.f, 50.f);

		bool bHasLOS = !GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams);
		if (bHasLOS)
		{
			// shoot
			if (InvComp->UseItem(WeaponSlot))
			{
				// remove weapon if out of ammo
				if (Inv[WeaponSlot]->GetValue() <= 0)
				{
					InvComp->RemoveItem(WeaponSlot);
					Controller->ClearFocus(EAIFocusPriority::Gameplay);
				}
				return EBTNodeResult::Succeeded;
			}
		}
		else
		{
			// Face the zombie but don't shoot if out of sight
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
