namespace AvaloniaUI.ViewModels;

/// <summary>
/// An interface for all Aspect View-Models.
/// </summary>
public interface IAspectViewModel
{
    /// <summary>
    /// Need this: it is the trigger for switching the view.
    /// </summary>
    ServiceAspectType ServiceAspectType { get; }
    
    /// <summary>
    /// A short description of what the Aspect does.
    /// </summary>
    string EditorDescription { get; }
}