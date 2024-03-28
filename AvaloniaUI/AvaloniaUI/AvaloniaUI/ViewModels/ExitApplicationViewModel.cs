using System.Reactive;
using System.Reactive.Linq;
using System.Threading.Tasks;
using System.Windows.Input;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class ExitApplicationViewModel : ViewModelBase
{
    public ExitApplicationViewModel()
    {
        ExitApplicationInteraction = new Interaction<Unit, bool?>();
        ExitApplicationCommand = ReactiveCommand.CreateFromTask(ExitApplication);
    }
    
    public Interaction<Unit, bool?> ExitApplicationInteraction { get; }

    // ReSharper disable once UnusedAutoPropertyAccessor.Global
    public ICommand ExitApplicationCommand { get; }

    private async Task ExitApplication()
    {
        await ExitApplicationInteraction.Handle(new Unit());
    }
}