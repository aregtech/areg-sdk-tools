using System.Collections.ObjectModel;
using System.IO;
using AvaloniaUI.ViewModels.Msg;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class ServiceTabItem(string openServiceFileFullPath) : ViewModelBase
{
    private ServiceTabViewModel _content = new(openServiceFileFullPath);
        
    public string Header { get; } = Path.GetFileName(openServiceFileFullPath);

    public ServiceTabViewModel Content
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
        EventAggregator.GetEvent<OpenServiceFullPathMsg>().Subscribe(OnNewServiceMessage);
    }

    private void OnNewServiceMessage(string openServiceFileFullPath)
    {
        var newTabItem = new ServiceTabItem(openServiceFileFullPath);
        ServiceTabItems.Add(newTabItem);
        SelectedServiceTabItem = newTabItem;
    }
}