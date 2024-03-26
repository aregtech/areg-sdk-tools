namespace AvaloniaUI.ViewModels;

public class MainViewModel : ViewModelBase
{
    // ReSharper disable once EmptyConstructor
    public MainViewModel()
    {
    }

    public string Greeting => "Welcome to Avalonia!";

    public OpenServiceViewModel OpenServiceInteraction { get; } = new();
}