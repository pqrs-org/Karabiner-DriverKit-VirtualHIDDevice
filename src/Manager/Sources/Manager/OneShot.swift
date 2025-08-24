/// OneShot is a small utility actor that behaves like a one-time "latch" or "promise".
///
/// Typical use case: wrapping delegate/callback style APIs into async/await.
/// - `wait()` suspends until a value or error is provided.
/// - `resume(_:)` provides that value or error exactly once.
///
/// Why the `pending` buffer?
/// In Swift Concurrency, ordering of tasks is not guaranteed. For example:
///   - `resume(_:)` might be called *before* `wait()` is registered,
///   - or `wait()` might be called *before* `resume(_:)`.
/// Without a buffer, the first case would drop the result, leaving the continuation hanging.
/// To solve this, OneShot stores a `pending` result if no continuation is waiting yet.
/// When `wait()` is eventually called, it immediately consumes the pending value.
/// Likewise, if `wait()` is called first, the continuation is stored and later resumed.
///
/// Safety:
/// - The actor guarantees serialized access to its state (`continuation`, `pending`).
/// - `resume(_:)` is idempotent: it will only deliver a result once.
/// - `T` and `E` are constrained to `Sendable` so results can safely cross concurrency domains.
///
/// In short: OneShot provides a safe, minimal mechanism to "bridge" from callback-based
/// APIs into async/await without race conditions when calls arrive in different orders.
public actor OneShot<T: Sendable, E: Error & Sendable> {
  private var continuation: CheckedContinuation<T, Error>?
  private var pending: Result<T, E>?

  func wait() async throws -> T {
    try await withCheckedThrowingContinuation { (c: CheckedContinuation<T, Error>) in
      if let p = pending {
        pending = nil
        c.resume(with: p.mapError { $0 as Error })
      } else {
        continuation = c
      }
    }
  }

  func resume(_ result: Result<T, E>) {
    if let c = continuation {
      continuation = nil
      c.resume(with: result.mapError { $0 as Error })
    } else {
      pending = result
    }
  }
}

extension OneShot {
  /// AnySendableError is intended for OneShot only.
  /// Do not use it for other purposes.
  public struct AnySendableError: Error, @unchecked Sendable {
    public let base: any Error
    public init(_ base: any Error) { self.base = base }
  }
}

public typealias OneShotAnyError<T: Sendable> = OneShot<T, OneShot<Never, Never>.AnySendableError>
