// AI/TTSClient.cpp
#include "AI/TTSClient.h"
#include "HAL/PlatformProcess.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

UTTSClient::UTTSClient()
{
}

void UTTSClient::SynthesizeAndPlay(const FString& Text)
{
    if (Text.IsEmpty()) return;
    
    FString EscapedText = Text.Replace(TEXT("'"), TEXT("''"));
    FString EscapedVoice = VoiceName.Replace(TEXT("'"), TEXT("''"));
    
    FString ScriptPath = FPaths::ProjectSavedDir() + TEXT("tts_temp.ps1");
    FString ScriptContent = FString::Printf(TEXT(
        "Add-Type -AssemblyName System.Speech\n"
        "$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer\n"
        "try { $synth.SelectVoice('%s') } catch { }\n"
        "$synth.Rate = %d\n"
        "$synth.Volume = %d\n"
        "$synth.Speak('%s')\n"
    ), *EscapedVoice, SpeechRate, SpeechVolume, *EscapedText);
    
    FFileHelper::SaveStringToFile(ScriptContent, *ScriptPath);
    
    FString Command = FString::Printf(TEXT("-ExecutionPolicy Bypass -File \"%s\""), *ScriptPath);
    
    FPlatformProcess::CreateProc(
        TEXT("powershell.exe"),
        *Command,
        false,
        true,
        true,
        nullptr,
        0,
        nullptr,
        nullptr
    );
    
    float Duration = FMath::Clamp(Text.Len() * 0.1f, 1.0f, 15.0f);
    OnSpeechStart.Broadcast(Duration);
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(SpeechTimerHandle);
        World->GetTimerManager().SetTimer(SpeechTimerHandle, [this]()
        {
            OnSpeechEnd.Broadcast();
        }, Duration + 0.5f, false);
    }
}

UWorld* UTTSClient::GetWorld() const
{
    if (HasAnyFlags(RF_ClassDefaultObject))
        return nullptr;
    
    if (UObject* Outer = GetOuter())
        return Outer->GetWorld();
    
    return nullptr;
}