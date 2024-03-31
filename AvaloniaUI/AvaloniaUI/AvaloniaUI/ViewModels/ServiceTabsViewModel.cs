using System;
using AvaloniaUI.ViewModels.Msg;

namespace AvaloniaUI.ViewModels;

public class ServiceTabsViewModel : ViewModelBase
{
    public ServiceTabsViewModel()
    {
        EventPublisher.GetEvent<OpenServiceMsg>().Subscribe(OnNewServiceMessage);
    }

    private static void OnNewServiceMessage(string openServiceFile)
    {
        Console.WriteLine($"New Service Opened: {openServiceFile}!");
    }
}