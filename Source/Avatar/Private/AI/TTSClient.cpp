#include "AI/TTSClient.h"
#include "HAL/PlatformProcess.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

UTTSClient::UTTSClient()
{
    UE_LOG(LogTemp, Warning, TEXT("TTSClient создан"));
}

void UTTSClient::SynthesizeAndPlay(const FString& Text)
{
    if (Text.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("TTS: Текст пустой!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TTS: Попытка озвучить текст: %s"), *Text);
    
    // Экранируем кавычки для PowerShell
    FString EscapedText = Text.Replace(TEXT("'"), TEXT("''"));
    FString EscapedVoice = VoiceName.Replace(TEXT("'"), TEXT("''"));
    
    // Создаем временный PS1 файл для отладки
    FString ScriptPath = FPaths::ProjectSavedDir() + TEXT("tts_debug.ps1");
    FString ScriptContent = FString::Printf(TEXT(
        "Write-Host 'TTS Script Started' -ForegroundColor Green\n"
        "Write-Host 'Text: %s' -ForegroundColor Yellow\n"
        "Write-Host 'Voice: %s' -ForegroundColor Yellow\n"
        "Add-Type -AssemblyName System.Speech\n"
        "$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer\n"
        "Write-Host 'Voices available:' -ForegroundColor Cyan\n"
        "$synth.GetInstalledVoices() | ForEach-Object { Write-Host $_.VoiceInfo.Name }\n"
        "try { \n"
        "    $synth.SelectVoice('%s')\n"
        "    Write-Host 'Voice selected successfully' -ForegroundColor Green\n"
        "} catch { \n"
        "    Write-Host 'Failed to select voice, using default' -ForegroundColor Red\n"
        "}\n"
        "$synth.Rate = %d\n"
        "$synth.Volume = %d\n"
        "Write-Host 'Speaking...' -ForegroundColor Green\n"
        "$synth.Speak('%s')\n"
        "Write-Host 'Speech completed' -ForegroundColor Green\n"
        "Start-Sleep -Seconds 1\n"
        ),
        *EscapedText,
        *EscapedVoice,
        *EscapedVoice,
        SpeechRate,
        SpeechVolume,
        *EscapedText
    );
    
    // Сохраняем скрипт
    FFileHelper::SaveStringToFile(ScriptContent, *ScriptPath);
    UE_LOG(LogTemp, Warning, TEXT("TTS: Скрипт сохранен в %s"), *ScriptPath);
    
    // Запускаем PowerShell с видимым окном для отладки
    FString Command = FString::Printf(
        TEXT("-NoExit -ExecutionPolicy Bypass -File \"%s\""),
        *ScriptPath
    );
    
    UE_LOG(LogTemp, Warning, TEXT("TTS: Запуск команды: powershell.exe %s"), *Command);
    
    // Запускаем с видимым окном
    FProcHandle ProcHandle = FPlatformProcess::CreateProc(
        TEXT("powershell.exe"),
        *Command,
        false,  
        false,  
        true,
        nullptr,
        0,
        nullptr,
        nullptr
    );
    
    if (ProcHandle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("TTS: Процесс PowerShell запущен успешно"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TTS: НЕ УДАЛОСЬ запустить PowerShell!"));
    }
    
    // Отправляем события для анимации
    float Duration = Text.Len() * 0.1f;
    OnSpeechStart.Broadcast(Duration);
    
    // Таймер для окончания речи
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(SpeechTimerHandle, [this]()
        {
            UE_LOG(LogTemp, Warning, TEXT("TTS: Таймер окончания речи сработал"));
            OnSpeechEnd.Broadcast();
        }, Duration, false);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TTS: World не найден, таймер не установлен"));
        OnSpeechEnd.Broadcast();
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