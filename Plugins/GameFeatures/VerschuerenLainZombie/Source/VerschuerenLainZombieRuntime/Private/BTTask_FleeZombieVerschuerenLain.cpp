#include "BTTask_FleeZombieVerschuerenLain.h"
#include "Survivor/SurvivorPawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"

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

	// calculate flee location opposite to the zombie
	FVector FleeDir = (PawnLoc - ZombieLoc).GetSafeNormal2D();
	if (FleeDir.IsNearlyZero())
	{
		FleeDir = Survivor->GetActorForwardVector();
	}

	FVector FleeTarget = PawnLoc + FleeDir * FleeDistance;

	// project target on NavMesh
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
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

	return EBTNodeResult::Succeeded;
}
