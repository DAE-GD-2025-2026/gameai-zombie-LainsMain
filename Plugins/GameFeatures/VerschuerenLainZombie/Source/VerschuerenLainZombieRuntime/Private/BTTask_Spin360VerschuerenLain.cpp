#include "BTTask_Spin360VerschuerenLain.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_Spin360VerschuerenLain::UBTTask_Spin360VerschuerenLain()
{
	NodeName = TEXT("Spin 360 VerschuerenLain");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Spin360VerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	bNotifyTick = true; // Force-enable ticking to bypass Unreal Engine CDO caching bugs!

	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (!Pawn) return EBTNodeResult::Failed;

	FBTSpin360MemoryVerschuerenLain* MyMemory = reinterpret_cast<FBTSpin360MemoryVerschuerenLain*>(NodeMemory);
	MyMemory->TotalRotated = 0.f;
	MyMemory->CurrentRotation = Pawn->GetActorRotation();

	if (Controller)
	{
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Spin360VerschuerenLain::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FBTSpin360MemoryVerschuerenLain* MyMemory = reinterpret_cast<FBTSpin360MemoryVerschuerenLain*>(NodeMemory);

	float YawDelta = SpinSpeed * DeltaSeconds;
	MyMemory->TotalRotated += YawDelta;

	MyMemory->CurrentRotation.Yaw += YawDelta;
	MyMemory->CurrentRotation.Normalize();

	Pawn->SetActorRotation(MyMemory->CurrentRotation);
	Controller->SetControlRotation(MyMemory->CurrentRotation);

	if (MyMemory->TotalRotated >= 360.f)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(Needs360ScanKey.SelectedKeyName, false);
		}
		
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
