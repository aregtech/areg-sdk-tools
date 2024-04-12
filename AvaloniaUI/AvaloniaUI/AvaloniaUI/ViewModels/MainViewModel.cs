namespace AvaloniaUI.ViewModels;

public class MainViewModel : ViewModelBase
{
    // ReSharper disable once ConvertConstructorToMemberInitializers
    public MainViewModel()
    {
        LoggerViewModel = new LoggerViewModel();
    }

    public OpenServiceViewModel OpenServiceInteraction { get; } = new();

    public ExitApplicationViewModel ExitApplicationInteraction { get; } = new();
    
    // ReSharper disable once UnusedAutoPropertyAccessor.Global
    public ILoggerViewModel LoggerViewModel { get; }
}