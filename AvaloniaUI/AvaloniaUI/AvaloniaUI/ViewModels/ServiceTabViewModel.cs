using System;
using System.Collections.ObjectModel;
using System.Linq;
using AvaloniaUI.Helpers;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

/// <summary>
/// The item for a single aspect of type <see cref="ViewModels.ServiceAspectType"/>.
/// It serves as the View-Model for the ServiceTabView.
/// It serves as the DataContext of each Aspect-View, which share one single <see cref="SharedAspectViewModel"/>.
/// </summary>
public class ServiceAspectTabItem(ServiceAspectType serviceAspectType, AspectViewModel aspectViewModel)
    : ViewModelBase
{
    public string Header { get; } = serviceAspectType.ToString();

    /// <summary>
    /// Need this: it is the trigger for switching the view, and is once per tab.
    /// </summary>
    public ServiceAspectType ServiceAspectType { get; } = serviceAspectType;

    /// <summary>
    /// The shared VM for all aspects.
    /// </summary>
    public AspectViewModel SharedAspectViewModel { get; } = aspectViewModel;
}

public class ServiceTabViewModel : ViewModelBase
{
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
        var dataSource = openServiceFileFullPath.ReadServiceFile();

        // we share one instance of ServiceAspectViewModel with many Views
        var serviceAspectViewModel = new AspectViewModel(dataSource);

        Enum.GetNames(typeof(ServiceAspectType))
            .ToList()
            .ForEach(x =>
                _serviceAspectTabItems
                    .Add(new ServiceAspectTabItem(
                        x.ToEnum<ServiceAspectType>(), serviceAspectViewModel)));

        SelectedServiceAspectTabItem = _serviceAspectTabItems.First();
    }
}