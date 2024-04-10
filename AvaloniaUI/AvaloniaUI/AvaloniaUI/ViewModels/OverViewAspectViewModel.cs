using System;
using XsdGen;

namespace AvaloniaUI.ViewModels;

public class OverViewAspectViewModel : AspectViewModelBase
{
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Overview;
    
    public override string EditorDescription { get; } = "Service Interface General Description";

    public OverViewAspectViewModel(ServiceInterface dataSource) : base(dataSource)
    {
        ServiceName = DataSource.Overview.Name;
        IsRemoteService = DataSource.Overview.IsRemote;
        
        var version = new Version(DataSource.Overview.Version);
        MajorVersion = version.Major;
        MinorVersion = version.Minor;
        PatchVersion = version.Build;
    }
    
    public string ServiceName { get; }
    
    public bool IsRemoteService { get; }
    
    public int MajorVersion { get; }

    public int MinorVersion { get; }
    
    public int PatchVersion { get; }
}