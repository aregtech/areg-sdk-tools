using System;
using AvaloniaUI.ViewModels.Msg;

namespace AvaloniaUI.ViewModels;

public class ServiceTabsViewModel : ViewModelBase
{
    private readonly ObservableMessageConsumer<OpenServiceMsg> _openServiceConsumer;

    public ServiceTabsViewModel()
    {
        _openServiceConsumer =
            new ObservableMessageConsumer<OpenServiceMsg>(
                OnNewServiceMessage,
                OnNewServiceCompleted);
    }

    private void OnNewServiceMessage(OpenServiceMsg msg)
    {
        Console.WriteLine($"New Service Opened: {msg.FullPath}!");
    }

    private void OnNewServiceCompleted()
    {
        Console.WriteLine($"New Service Completed! Disposing ...");
        _openServiceConsumer.Dispose();
    }
}