#include "UI/ChatWidget.h"
#include "Core/AvatarGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"

void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Warning, TEXT("=== ChatWidget::NativeConstruct ==="));
    
    // Получаем GameInstance
    UWorld* World = GetWorld();
    if (World)
    {
        GameInstance = Cast<UAvatarGameInstance>(World->GetGameInstance());
        UE_LOG(LogTemp, Warning, TEXT("GameInstance from World: %p"), GameInstance);
    }
    
    if (GameInstance)
    {
        GameInstance->OnChatMessageReceived.AddDynamic(this, &UChatWidget::OnUserMessageReceived);
        GameInstance->OnAvatarResponse.AddDynamic(this, &UChatWidget::OnAvatarResponseReceived);
        UE_LOG(LogTemp, Warning, TEXT("Successfully bound to GameInstance delegates"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Failed to get GameInstance in ChatWidget!"));
    }
    
    // Проверяем элементы UI
    if (MessageInputBox)
    {
        MessageInputBox->OnTextCommitted.AddDynamic(this, &UChatWidget::OnMessageCommitted);
        UE_LOG(LogTemp, Warning, TEXT("MessageInputBox found"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MessageInputBox is NULL! Check name in Blueprint"));
    }
    
    if (SendButton)
    {
        SendButton->OnClicked.AddDynamic(this, &UChatWidget::OnSendMessage);
        UE_LOG(LogTemp, Warning, TEXT("SendButton found"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SendButton is NULL! Check name in Blueprint"));
    }
    
    if (ChatHistoryBox)
    {
        UE_LOG(LogTemp, Warning, TEXT("ChatHistoryBox found"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ChatHistoryBox is NULL! Check name in Blueprint"));
    }
    
    // Добавляем приветственное сообщение
    AddMessage(TEXT("Система"), TEXT("Здравствуйте! Я виртуальный ассистент. Чем я могу вам помочь?"));
}

void UChatWidget::OnSendMessage()
{
    UE_LOG(LogTemp, Warning, TEXT("=== OnSendMessage CALLED ==="));
    
    if (!MessageInputBox)
    {
        UE_LOG(LogTemp, Error, TEXT("MessageInputBox is NULL!"));
        return;
    }
    
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance is NULL!"));
        return;
    }
    
    FString Message = MessageInputBox->GetText().ToString();
    UE_LOG(LogTemp, Warning, TEXT("Message text: %s"), *Message);
    
    if (Message.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Message is empty, ignoring"));
        return;
    }
    
    // Добавляем сообщение пользователя в чат
    AddMessage(TEXT("Вы"), Message);
    
    // Отправляем в GameInstance
    UE_LOG(LogTemp, Warning, TEXT("Calling SendUserMessage..."));
    GameInstance->SendUserMessage(Message);
    
    // Очищаем поле ввода
    MessageInputBox->SetText(FText::GetEmpty());
}

void UChatWidget::OnMessageCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    UE_LOG(LogTemp, Warning, TEXT("OnMessageCommitted: CommitMethod=%d"), CommitMethod);
    
    if (CommitMethod == ETextCommit::OnEnter)
    {
        OnSendMessage();
    }
}

void UChatWidget::OnUserMessageReceived(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("OnUserMessageReceived: %s"), *Message);
    // Сообщение уже добавлено при отправке, поэтому здесь ничего не делаем
    // Можно добавить индикатор "печатает" если нужно
}

void UChatWidget::OnAvatarResponseReceived(const FString& Response)
{
    UE_LOG(LogTemp, Warning, TEXT("OnAvatarResponseReceived: %s"), *Response);
    
    FString SenderName = GameInstance ? GameInstance->AvatarName : TEXT("Ассистент");
    AddMessage(SenderName, Response);
}

void UChatWidget::AddMessage(const FString& Sender, const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("AddMessage: %s - %s"), *Sender, *Message);
    
    if (!ChatHistoryBox)
    {
        UE_LOG(LogTemp, Error, TEXT("AddMessage: ChatHistoryBox is NULL!"));
        return;
    }
    
    if (!MessageWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AddMessage: MessageWidgetClass is NULL!"));
        return;
    }
    
    // Создаем виджет сообщения
    UUserWidget* MessageWidget = CreateWidget<UUserWidget>(this, MessageWidgetClass);
    if (!MessageWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("AddMessage: Failed to create MessageWidget!"));
        return;
    }
    
    // Находим текстовые поля в виджете сообщения
    UTextBlock* SenderText = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("SenderText")));
    UTextBlock* MessageText = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("MessageText")));
    
    if (SenderText)
    {
        SenderText->SetText(FText::FromString(Sender));
        UE_LOG(LogTemp, Warning, TEXT("SenderText set: %s"), *Sender);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SenderText not found in MessageWidget!"));
    }
    
    if (MessageText)
    {
        MessageText->SetText(FText::FromString(Message));
        UE_LOG(LogTemp, Warning, TEXT("MessageText set: %s"), *Message);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MessageText not found in MessageWidget!"));
    }
    
    // Добавляем в ChatHistoryBox
    UScrollBoxSlot* NewMessageSlot = Cast<UScrollBoxSlot>(ChatHistoryBox->AddChild(MessageWidget));
    if (NewMessageSlot)
    {
        NewMessageSlot->SetPadding(FMargin(0, 5, 0, 5));
        UE_LOG(LogTemp, Warning, TEXT("Message added to ChatHistoryBox"));
    }
    
    // Прокручиваем вниз
    ChatHistoryBox->ScrollToEnd();
}