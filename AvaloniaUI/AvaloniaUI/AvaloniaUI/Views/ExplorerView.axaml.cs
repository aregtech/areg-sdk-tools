using Avalonia.ReactiveUI;
using AvaloniaUI.ViewModels;

namespace AvaloniaUI.Views;

public partial class ExplorerView : ReactiveUserControl<ExplorerViewModel>
{
    public ExplorerView()
    {
        InitializeComponent();
    }
}