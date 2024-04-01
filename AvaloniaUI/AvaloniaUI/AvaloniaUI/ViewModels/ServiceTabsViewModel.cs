using System.Collections.ObjectModel;
using AvaloniaUI.ViewModels.Msg;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class ServiceTabItem(string header) : ViewModelBase
{
    private SingleServiceViewModel _content = new();
        
    public string Header { get; } = header;

    public SingleServiceViewModel Content
    {
        get => _content;
        set => this.RaiseAndSetIfChanged(ref _content, value);
    }
}

public class ServiceTabsViewModel : ViewModelBase
{
    private ObservableCollection<ServiceTabItem> _serviceTabItems;
    private ServiceTabItem? _selectedServiceTabItem;
    
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
        var newTabItem = new ServiceTabItem(openServiceFile);
        ServiceTabItems.Add(newTabItem);
        SelectedServiceTabItem = newTabItem;
    }
}