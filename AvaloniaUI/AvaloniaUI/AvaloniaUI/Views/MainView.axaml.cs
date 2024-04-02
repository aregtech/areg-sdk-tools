using System.Linq;
using System.Reactive;
using System.Reactive.Disposables;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Platform.Storage;
using Avalonia.ReactiveUI;
using AvaloniaUI.ViewModels;
using MsBox.Avalonia;
using MsBox.Avalonia.Enums;
using ReactiveUI;

namespace AvaloniaUI.Views;

public partial class MainView : ReactiveUserControl<MainViewModel>
{
    private bool _shouldCloseApp; // this guards against shutting down the App twice (event comes twice)
    
    public MainView()
    {
        InitializeComponent();
        
        this
            .WhenActivated(disposables =>
            {
                this.ViewModel?
                    .OpenServiceInteraction.OpenServiceFileInteraction
                    .RegisterHandler(HandleOpenServiceInteraction)
                    .DisposeWith(disposables);

                this.ViewModel?
                    .ExitApplicationInteraction.ExitApplicationInteraction
                    .RegisterHandler(HandleExitApplicationInteraction)
                    .DisposeWith(disposables);
            });
    }
    
    private async Task HandleOpenServiceInteraction(InteractionContext<string?, string[]?> context)
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

        context.SetOutput(storageFiles.Select(x => x.Path.ToString()).ToArray());
    }
    
    private async Task HandleExitApplicationInteraction(InteractionContext<Unit, bool?> context)
    {
        if (!_shouldCloseApp)
        {
            var box = MessageBoxManager
                .GetMessageBoxStandard(
                    title: "Application Shutdown",
                    text: "Do you wish to first save any open services?",
                    ButtonEnum.YesNo,
                    Icon.Stop);

            var topLevel = TopLevel.GetTopLevel(this);
            var result = await box.ShowWindowDialogAsync(topLevel as Window);
            
            _shouldCloseApp = result == ButtonResult.Yes;
            if (_shouldCloseApp)
            {
                FinalApplicationShutdown();
            }
        }

        // this terminates the interaction
        context.SetOutput(_shouldCloseApp);
    }
    
    private static void FinalApplicationShutdown()
    {
        switch (Application.Current?.ApplicationLifetime)
        {
            case IClassicDesktopStyleApplicationLifetime desktopApp:
                desktopApp.Shutdown();
                break;
            case ISingleViewApplicationLifetime viewApp:
                viewApp.MainView = null;
                break;
        }
    }
}