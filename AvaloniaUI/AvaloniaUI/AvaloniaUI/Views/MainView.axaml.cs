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
using AvaloniaUI.ViewModels.Msg;
using MsBox.Avalonia;
using MsBox.Avalonia.Enums;
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

        context.SetOutput(storageFiles.Select(x => x.Name).ToArray());
    }
    
    private async Task HandleExitApplicationInteraction(InteractionContext<Unit, bool?> context)
    {
        var box = MessageBoxManager
            .GetMessageBoxStandard(
                title: "Application Shutdown",
                text: "Do you wish to first save any open services?",
                ButtonEnum.YesNo,
                Icon.Stop);

        var topLevel = TopLevel.GetTopLevel(this);
        var result = await box.ShowWindowDialogAsync(topLevel as Window);
        var shouldCloseApp = result == ButtonResult.Yes;
        context.SetOutput(shouldCloseApp);

        if (shouldCloseApp)
        {
            ShutdownMessageProducers();
            FinalApplicationShutdown();
        }
    }

    private void ShutdownMessageProducers()
    {
        // notify message observables to Dispose themselves (yuk)
        // var allMessageObservables
        //     = AppContainer.Instance.GetAllInstances<ObservableMessageProducer<OpenServiceMsg>>();
        //
        // foreach (var observableMessage in allMessageObservables)
        // {
        //     observableMessage.OnCompleted();
        // }

        var producer = AppContainer.Instance.GetInstance<ObservableMessageProducer<OpenServiceMsg>>();
        producer.OnCompleted();
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