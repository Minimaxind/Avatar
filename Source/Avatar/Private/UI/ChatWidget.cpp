#include "UI/ChatWidget.h"
#include "Core/AvatarGameInstance.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"

void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UWorld* World = GetWorld();
    if (World)
    {
        GameInstance = Cast<UAvatarGameInstance>(World->GetGameInstance());
    }
    
    if (GameInstance)
    {
        GameInstance->OnChatMessageReceived.AddDynamic(this, &UChatWidget::OnUserMessageReceived);
        GameInstance->OnAvatarResponse.AddDynamic(this, &UChatWidget::OnAvatarResponseReceived);
    }
    
    if (MessageInputBox)
    {
        MessageInputBox->OnTextCommitted.AddDynamic(this, &UChatWidget::OnMessageCommitted);
    }
    
    if (SendButton)
    {
        SendButton->OnClicked.AddDynamic(this, &UChatWidget::OnSendMessage);
    }
    
    AddMessage(TEXT("Система"), TEXT("Здравствуйте! Я виртуальный ассистент. Чем я могу вам помочь?"));
}

void UChatWidget::OnSendMessage()
{
    if (!MessageInputBox || !GameInstance) return;
    
    FString Message = MessageInputBox->GetText().ToString();
    if (Message.IsEmpty()) return;
    
    AddMessage(TEXT("Вы"), Message);
    GameInstance->SendUserMessage(Message);
    MessageInputBox->SetText(FText::GetEmpty());
}

void UChatWidget::OnMessageCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        OnSendMessage();
    }
}

void UChatWidget::OnUserMessageReceived(const FString& Message)
{
}

void UChatWidget::OnAvatarResponseReceived(const FString& Response)
{
    FString SenderName = GameInstance ? GameInstance->AvatarName : TEXT("Ассистент");
    AddMessage(SenderName, Response);
}

void UChatWidget::AddMessage(const FString& Sender, const FString& Message)
{
    if (!ChatHistoryBox || !MessageWidgetClass) return;
    
    UUserWidget* MessageWidget = CreateWidget<UUserWidget>(this, MessageWidgetClass);
    if (!MessageWidget) return;
    
    UTextBlock* SenderText = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("SenderText")));
    UTextBlock* MessageText = Cast<UTextBlock>(MessageWidget->GetWidgetFromName(TEXT("MessageText")));
    
    if (SenderText) SenderText->SetText(FText::FromString(Sender));
    if (MessageText) MessageText->SetText(FText::FromString(Message));
    
    UScrollBoxSlot* NewMessageSlot = Cast<UScrollBoxSlot>(ChatHistoryBox->AddChild(MessageWidget));
    if (NewMessageSlot) NewMessageSlot->SetPadding(FMargin(0, 5, 0, 5));
    
    ChatHistoryBox->ScrollToEnd();
}