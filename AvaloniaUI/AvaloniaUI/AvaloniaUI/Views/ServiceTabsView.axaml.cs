using Avalonia.ReactiveUI;
using AvaloniaUI.ViewModels;

namespace AvaloniaUI.Views;

public partial class ServiceTabsView : ReactiveUserControl<ServiceTabsViewModel>
{
    public ServiceTabsView()
    {
        InitializeComponent();
    }
}