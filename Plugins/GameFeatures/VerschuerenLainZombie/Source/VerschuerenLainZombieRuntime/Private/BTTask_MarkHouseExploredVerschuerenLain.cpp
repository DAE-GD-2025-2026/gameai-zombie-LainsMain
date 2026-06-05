#include "BTTask_MarkHouseExploredVerschuerenLain.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "Village/House/House.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UBTTask_MarkHouseExploredVerschuerenLain::UBTTask_MarkHouseExploredVerschuerenLain()
{
	NodeName = TEXT("Mark House Explored VerschuerenLain");
}

EBTNodeResult::Type UBTTask_MarkHouseExploredVerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UZombieSurvMemoryComponentVerschuerenLain* MemoryComp = Pawn->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>();
	if (!MemoryComp) return EBTNodeResult::Failed;

	FVector PawnLoc = Pawn->GetActorLocation();

	// mark closest house explored (optimized)
	MemoryComp->MarkClosestHouseExplored(PawnLoc, 350.f);

	return EBTNodeResult::Succeeded;
}
