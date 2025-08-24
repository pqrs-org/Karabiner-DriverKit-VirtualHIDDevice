import Foundation
import SystemExtensions

class ExtensionManager: NSObject, OSSystemExtensionRequestDelegate {
  enum Operation {
    case activate
    case forceActivate
    case deactivate
  }

  private let bundleIdentifier = "org.pqrs.Karabiner-DriverKit-VirtualHIDDevice"
  private let operation: Operation
  private let oneShot = OneShotAnyError<OSSystemExtensionRequest.Result>()

  //
  // Actions
  //

  static func perform(_ operation: Operation) async throws -> OSSystemExtensionRequest.Result {
    let manager = ExtensionManager(operation)
    return try await manager.submitRequest()
  }

  private init(_ operation: Operation) {
    self.operation = operation
  }

  private func submitRequest() async throws -> OSSystemExtensionRequest.Result {
    let request: OSSystemExtensionRequest

    switch operation {
    case .activate, .forceActivate:
      request = OSSystemExtensionRequest.activationRequest(
        forExtensionWithIdentifier: bundleIdentifier,
        queue: .main
      )
      print("activation of \(bundleIdentifier) is requested")

    case .deactivate:
      request = OSSystemExtensionRequest.deactivationRequest(
        forExtensionWithIdentifier: bundleIdentifier,
        queue: .main
      )
      print("deactivation of \(bundleIdentifier) is requested")
    }

    request.delegate = self

    OSSystemExtensionManager.shared.submitRequest(request)

    return try await oneShot.wait()
  }

  //
  // OSSystemExtensionRequestDelegate
  //

  func request(
    _ request: OSSystemExtensionRequest,
    didFinishWithResult result: OSSystemExtensionRequest.Result
  ) {
    print("request of \(request.identifier) is finished")

    switch result {
    case .completed:
      print("request of \(request.identifier) is completed")
    case .willCompleteAfterReboot:
      print("request of \(request.identifier) requires reboot")
    @unknown default:
      break
    }

    let shot = self.oneShot
    Task { await shot.resume(.success(result)) }
  }

  func request(
    _ request: OSSystemExtensionRequest,
    didFailWithError error: Error
  ) {
    print("request of \(request.identifier) is failed with error: \(error.localizedDescription)")

    let operation = self.operation
    let shot = self.oneShot
    Task {
      if operation == .deactivate, (error as? OSSystemExtensionError)?.code == .extensionNotFound {
        // Ignorable errors
        await shot.resume(.success(.completed))
      } else {
        await shot.resume(.failure(OneShot.AnySendableError(error)))
      }
    }
  }

  func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {
    print("request of \(request.identifier) requires user approval")
  }

  func request(
    _ request: OSSystemExtensionRequest,
    actionForReplacingExtension existing: OSSystemExtensionProperties,
    withExtension ext: OSSystemExtensionProperties
  ) -> OSSystemExtensionRequest.ReplacementAction {
    let existingVersion = existing.bundleVersion
    let extVersion = ext.bundleVersion

    if extVersion.compare(existingVersion, options: .numeric) == .orderedSame {
      print("\(request.identifier) \(extVersion) is already installed")
      return .cancel
    }

    if operation == .forceActivate {
      print("\(request.identifier) will be force replaced to \(extVersion) forcely")
      return .replace
    }

    if extVersion.compare(existingVersion, options: .numeric) == .orderedDescending {
      print("\(request.identifier) will be replaced to \(extVersion) from \(existingVersion)")
      return .replace
    }

    print(
      "request of \(request.identifier) is canceled because newer version (\(existingVersion)) is already installed"
    )
    return .cancel
  }
}
