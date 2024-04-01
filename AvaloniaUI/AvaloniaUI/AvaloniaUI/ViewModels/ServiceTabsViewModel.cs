using System.Collections.ObjectModel;
using AvaloniaUI.ViewModels.Msg;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class ServiceTabsViewModel : ViewModelBase
{
    private ObservableCollection<ServiceTabItem> _serviceTabItems;
    private ServiceTabItem? _selectedServiceTabItem;
    
    public class ServiceTabItem(string header, string content)
    {
        public string Header { get; } = header;
        public string Content { get; } = content;
    }

    public ObservableCollection<ServiceTabItem> ServiceTabItems
    {
        get => _serviceTabItems;
        set => this.RaiseAndSetIfChanged(ref _serviceTabItems, value);
    }

    public ServiceTabItem? SelectedServiceTabItem
    {
        get => _selectedServiceTabItem;
        set => this.RaiseAndSetIfChanged(ref _selectedServiceTabItem, value);
    }
        
    public ServiceTabsViewModel()
    {
        _serviceTabItems = new ObservableCollection<ServiceTabItem>();
        EventPublisher.GetEvent<OpenServiceMsg>().Subscribe(OnNewServiceMessage);
    }

    private void OnNewServiceMessage(string openServiceFile)
    {
        // Console.WriteLine($"New Service Opened: {openServiceFile}!");
        var newTabItem = new ServiceTabItem(openServiceFile, $"Showing Tab Number {_tabNum++}");
        ServiceTabItems.Add(newTabItem);
        SelectedServiceTabItem = newTabItem;
    }

    private static int _tabNum = 1;
}