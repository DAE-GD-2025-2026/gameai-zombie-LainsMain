#include "BTTask_FleeZombieVerschuerenLain.h"
#include "Survivor/SurvivorPawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "Items/BaseItem.h"
#include "Village/House/House.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FleeZombieVerschuerenLain::UBTTask_FleeZombieVerschuerenLain()
{
	NodeName = TEXT("Flee Zombie VerschuerenLain");
}

EBTNodeResult::Type UBTTask_FleeZombieVerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!Controller || !BlackboardComp) return EBTNodeResult::Failed;

	ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Controller->GetPawn());
	if (!Survivor) return EBTNodeResult::Failed;

	AActor* ZombieActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetZombieKey.SelectedKeyName));
	if (!ZombieActor) return EBTNodeResult::Failed;

	FVector PawnLoc = Survivor->GetActorLocation();
	FVector ZombieLoc = ZombieActor->GetActorLocation();

	FVector FleeTarget = PawnLoc;
	bool bTargetSet = false;

	UZombieSurvMemoryComponentVerschuerenLain* MemoryComp = Survivor->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>();

	if (MemoryComp)
	{
		// 1. Try to find a known weapon (Pistol or Shotgun) in memory to flee to
		FVector WeaponLoc = FVector::ZeroVector;
		ABaseItem* WeaponItem = nullptr;
		bool bFoundWeapon = false;

		// Check Shotgun first, then Pistol
		bFoundWeapon = MemoryComp->FindClosestItem(PawnLoc, EItemType::Shotgun, WeaponLoc, WeaponItem);
		if (!bFoundWeapon)
		{
			bFoundWeapon = MemoryComp->FindClosestItem(PawnLoc, EItemType::Pistol, WeaponLoc, WeaponItem);
		}

		if (bFoundWeapon)
		{
			FleeTarget = WeaponLoc;
			bTargetSet = true;
		}
		else
		{
			// 2. If no weapon known, try to find an unexplored house
			FVector HouseLoc = FVector::ZeroVector;
			AHouse* TargetHouse = nullptr;
			if (MemoryComp->GetNextUnexploredHouse(PawnLoc, HouseLoc, TargetHouse))
			{
				FleeTarget = HouseLoc;
				bTargetSet = true;
			}
			else
			{
				// 3. Fallback to the closest explored house
				TArray<AActor*> FoundHouses;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHouse::StaticClass(), FoundHouses);
				float MinDist = FLT_MAX;
				AHouse* ClosestHouse = nullptr;

				for (AActor* Actor : FoundHouses)
				{
					AHouse* House = Cast<AHouse>(Actor);
					if (House)
					{
						float Dist = FVector::Dist(PawnLoc, House->GetBounds().Origin);
						if (Dist < MinDist)
						{
							MinDist = Dist;
							ClosestHouse = House;
						}
					}
				}

				if (ClosestHouse)
				{
					FleeTarget = ClosestHouse->GetBounds().Origin;
					bTargetSet = true;
				}
			}
		}
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	// If we couldn't find a gun or a house, use the original hiding spot / directional flee
	if (!bTargetSet)
	{
		bool bFoundHidingPoint = false;
		if (NavSys)
		{
			// check 8 directions for a spot behind a wall
			for (int i = 0; i < 8; ++i)
			{
				float AngleRad = FMath::DegreesToRadians(i * 45.f);
				FVector Dir(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f);
				FVector Candidate = PawnLoc + Dir * FleeDistance;

				FNavLocation NavLoc;
				if (NavSys->ProjectPointToNavigation(Candidate, NavLoc))
				{
					FVector NavPoint = NavLoc.Location;

					// trace to check if wall blocks sight from zombie
					FHitResult HitResult;
					FCollisionQueryParams TraceParams;
					TraceParams.AddIgnoredActor(Survivor);
					TraceParams.AddIgnoredActor(ZombieActor);

					FVector StartTrace = ZombieLoc + FVector(0.f, 0.f, 100.f);
					FVector EndTrace = NavPoint + FVector(0.f, 0.f, 100.f);

					if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams))
					{
						// hit obstacle -> wall blocks sight!
						FleeTarget = NavPoint;
						bFoundHidingPoint = true;
						bTargetSet = true;
						break;
					}
				}
			}
		}

		// fallback if no hiding spot
		if (!bFoundHidingPoint)
		{
			FVector FleeDir = (PawnLoc - ZombieLoc).GetSafeNormal2D();
			if (FleeDir.IsNearlyZero())
			{
				FleeDir = Survivor->GetActorForwardVector();
			}
			FleeTarget = PawnLoc + FleeDir * FleeDistance;
			bTargetSet = true;
		}
	}

	// project target to NavMesh if set
	if (bTargetSet && NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(FleeTarget, NavLoc))
		{
			FleeTarget = NavLoc.Location;
		}
	}

	// update blackboard target location
	BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, FleeTarget);

	// sprint away
	Survivor->StartRunning();

	Controller->ClearFocus(EAIFocusPriority::Gameplay);

	return EBTNodeResult::Succeeded;
}
