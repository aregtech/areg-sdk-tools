using System;
using System.Collections.ObjectModel;
using System.Linq;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class ServiceAspectTabItem(ServiceAspectType serviceAspectType) : ViewModelBase
{
    public string Header { get; } = serviceAspectType.ToString();
    public ServiceAspectType Content { get; } = serviceAspectType;
}

public class ServiceTabViewModel : ViewModelBase
{
    private readonly string _openServiceFileFullPath;
    private ObservableCollection<ServiceAspectTabItem> _serviceAspectTabItems = [];
    private ServiceAspectTabItem? _selectedServiceAspectTabItem;

    public ObservableCollection<ServiceAspectTabItem> ServiceAspectTabItems
    {
        get => _serviceAspectTabItems;
        set => this.RaiseAndSetIfChanged(ref _serviceAspectTabItems, value);
    }

    public ServiceAspectTabItem? SelectedServiceAspectTabItem
    {
        get => _selectedServiceAspectTabItem;
        set => this.RaiseAndSetIfChanged(ref _selectedServiceAspectTabItem, value);
    }
    
    public ServiceTabViewModel(string openServiceFileFullPath)
    {
        _openServiceFileFullPath = openServiceFileFullPath;
        
        foreach (var aspect in Enum.GetNames(typeof(ServiceAspectType)))
        {
            Enum.TryParse<ServiceAspectType>(aspect, out var aspectEnum);
            _serviceAspectTabItems.Add(new ServiceAspectTabItem(aspectEnum));
        }

        SelectedServiceAspectTabItem = _serviceAspectTabItems.First();
    }
}