using System;
using System.Reactive.Linq;
using System.Reactive.Subjects;

namespace AvaloniaUI.ViewModels.Msg
{
    /// <summary>
    /// Just a tag interface.
    /// This facilitates retrieving all <see cref="ObservableMessageProducer{TMsg}"/> from the container regardless of
    /// the generic parameter.
    /// </summary>
    public interface IObservableMessageProducer<out TMsg>
        where TMsg : struct
    {
        IObservable<TMsg> GetObservable();

        void OnCompleted();
    }

    /// <summary>
    /// This is my version of an EventAggregator, except we use Rx and the container.
    /// This is the producer of messages.
    /// Each producer is essentially a singleton and made unique
    /// by its <see cref="TMsg"/> generic class parameter.
    /// To accomplish this, each <see cref="ObservableMessageProducer{TMsg}"/> pair must be registered with the container.
    /// 
    /// Got some help here: <see href="https://rehansaeed.com/reactive-extensions-part1-replacing-events/"/>
    /// </summary>
    public class ObservableMessageProducer<TMsg> : IObservableMessageProducer<TMsg>
        where TMsg : struct
    {
        private readonly Subject<TMsg> _subject = new();

        public IObservable<TMsg> GetObservable() => _subject.AsObservable();

        public void OnCompleted() => _subject.OnCompleted();

        public void NextMessage(TMsg args) => _subject.OnNext(args);
    }

    /// <summary>
    /// This is my version of an EventAggregator, except we use Rx and the container.
    /// This is the consumer of messages.
    /// There can be multiple consumers for each <see cref="ObservableMessageProducer{TMsg}"/> pair.
    /// </summary>
    public sealed class ObservableMessageConsumer<TMsg> : IDisposable
        where TMsg : struct
    {
        private readonly IDisposable _subscription;

        public ObservableMessageConsumer(Action<TMsg> onNext, Action? onCompleted = null)
        {
            var observableMessage = AppContainer.Instance.GetInstance<ObservableMessageProducer<TMsg>>();

            _subscription = onCompleted != null
                ? observableMessage.GetObservable().Subscribe(onNext, onCompleted)
                : observableMessage.GetObservable().Subscribe(onNext);
        }

        public void Dispose()
        {
            _subscription.Dispose();
        }
    }
}