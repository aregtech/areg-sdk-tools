using Avalonia.ReactiveUI;
using AvaloniaUI.ViewModels;

namespace AvaloniaUI.Views;

public partial class ServiceTabView : ReactiveUserControl<ServiceTabViewModel>
{
    public ServiceTabView()
    {
        InitializeComponent();
    }
}