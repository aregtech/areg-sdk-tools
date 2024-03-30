namespace AvaloniaUI.ViewModels.Msg;

/// <summary>
/// This message is ticked upon an "Open Service" event.
/// </summary>
/// <param name="FullPath">The full path of the service to open.</param>
public readonly record struct OpenServiceMsg(string FullPath);

/// <summary>
/// This message is ticked upon a "Close Application" event.
/// </summary>
// public readonly record struct CloseApplication;