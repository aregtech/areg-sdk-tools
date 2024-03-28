using System.Linq;
using System.Reactive.Disposables;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Platform.Storage;
using Avalonia.ReactiveUI;
using AvaloniaUI.ViewModels;
using ReactiveUI;

namespace AvaloniaUI.Views;

public partial class MainView : ReactiveUserControl<MainViewModel>
{
    public MainView()
    {
        InitializeComponent();
        
        this
            .WhenActivated(disposables =>
            {
                this.ViewModel?
                    .OpenServiceInteraction
                    .SelectServiceFileInteraction
                    .RegisterHandler(this.OpenServiceInteractionHandler)
                    .DisposeWith(disposables);
            });
    }
    
    private async Task OpenServiceInteractionHandler(InteractionContext<string?, string[]?> context)
    {
        // Get our parent top level control in order to get the needed service (in our sample the storage provider. Can also be the clipboard etc.)
        var topLevel = TopLevel.GetTopLevel(this);
        
        var storageFiles = await topLevel!.StorageProvider
            .OpenFilePickerAsync(
                new FilePickerOpenOptions
                {
                    AllowMultiple = true,
                    Title = context.Input,
                    FileTypeFilter = new FilePickerFileType[]
                    {
                        new("Service Interfaces")
                        {
                            Patterns = new[] { "*.siml" }
                        }
                    }
                });
               
        context.SetOutput(storageFiles.Select(x => x.Name).ToArray());
    }
}