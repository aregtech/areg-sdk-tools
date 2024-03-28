using Avalonia.Controls;
using AvaloniaUI.ViewModels;

namespace AvaloniaUI.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();

        this.Closing += HandleClosingEvent;
    }

    /// <summary>
    /// Handle the main window's closing event.
    /// </summary>
    /// Note that this comes twice, and, the 2nd time the event is, again, uncanceled.
    /// <param name="sender"></param>
    /// <param name="e"></param>
    private static void HandleClosingEvent(object? sender, WindowClosingEventArgs e)
    {
        e.Cancel = true;

        if (sender is not MainWindow mainWindow) return;
        if (mainWindow.DataContext is MainViewModel mainViewModel)
        {
            mainViewModel
                .ExitApplicationInteraction.ExitApplicationCommand
                .Execute(null);
        }
    }
}