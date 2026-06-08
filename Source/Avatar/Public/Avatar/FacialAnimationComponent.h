// Avatar/FacialAnimationComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FacialAnimationComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AVATAR_API UFacialAnimationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFacialAnimationComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StartSpeaking(const FString& Text, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StopSpeaking();

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    bool IsSpeaking() const { return bIsSpeaking; }

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetEmotion(const FString& Emotion, float Intensity = 1.0f);

    // -------------------------------------------------------
    // Имя Slot ноды в ABP_Face_PostProcess → AnimGraph
    // Должно совпадать с тем что ты добавишь в ABP
    // -------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    FName VisemeSlotName = FName("VisemeSlot");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float VisemeBlendIn = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float VisemeBlendOut = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float VisemePlayRate = 1.0f;

    // -------------------------------------------------------
    // Твои виземы — назначаются в BP_MetaHuman_Avatarus
    // в Details панели компонента FacialAnimation
    // -------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_A1;       // а, я

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_O1;       // о

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Yo1;      // ё

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_U1;       // у, ю

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_E1;       // э, е

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_I1;       // и, ы, й

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_B1;       // б, п

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_V1;       // в, ф

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_M1;       // м

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_T1;       // т, д, н

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_S1;       // с, з, ц

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Sh1;      // ш, щ, ж

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Ch1;      // ч

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_K1;       // к, г, х

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_L1;       // л

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_R1;       // р

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Idle;     // покой (рот закрыт)

private:
    UPROPERTY()
    USkeletalMeshComponent* FaceMeshComponent;

    TMap<TCHAR, UAnimSequence*> CharToAnimMap;
    TArray<UAnimSequence*> VisemeQueue;

    bool  bIsSpeaking         = false;
    float CurrentSpeechTime   = 0.0f;
    float TotalSpeechDuration = 0.0f;
    int32 CurrentVisemeIndex  = 0;
    float CurrentVisemeDuration = 0.1f;
    float TimeInCurrentViseme = 0.0f;

    // Моргание
    float TimeSinceLastBlink = 0.0f;
    float NextBlinkTime      = 0.0f;
    bool  bIsBlinking        = false;
    float BlinkProgress      = 0.0f;

    void BuildCharMap();
    void PlayVisemeAtIndex(int32 Index);
    void UpdateBlinking(float DeltaTime);
};