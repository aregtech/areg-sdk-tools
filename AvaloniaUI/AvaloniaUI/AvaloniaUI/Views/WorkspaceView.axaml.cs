using Avalonia.ReactiveUI;
using AvaloniaUI.ViewModels;

namespace AvaloniaUI.Views;

public partial class WorkspaceView : ReactiveUserControl<WorkspaceViewModel>
{
    public WorkspaceView()
    {
        InitializeComponent();
    }
}