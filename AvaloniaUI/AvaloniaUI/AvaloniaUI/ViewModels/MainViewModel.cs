namespace AvaloniaUI.ViewModels;

public class MainViewModel : ViewModelBase
{
    // ReSharper disable once EmptyConstructor
    public MainViewModel()
    {
    }

    public OpenServiceViewModel OpenServiceInteraction { get; } = new();

    public ExitApplicationViewModel ExitApplicationInteraction { get; } = new();
}