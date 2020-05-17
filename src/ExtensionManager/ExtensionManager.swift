import Foundation
import SystemExtensions

class ExtensionManager: NSObject, OSSystemExtensionRequestDelegate {
    static let shared = ExtensionManager()

    //
    // Notification names
    //

    static let stateChanged = Notification.Name("stateChanged")

    public enum State {
        case activationRequested
        case deactivationRequested
        case requestFinished
        case requestFailed
        case replacingCanceled
        case willReplace
        case userApprovalRequired
        case rebootRequired
    }

    struct NotificationObject {
        let bundleIdentifier: String
        let state: State
        let message: String
    }

    func activate(_ bundleIdentifier: String) {
        let request = OSSystemExtensionRequest.activationRequest(
            forExtensionWithIdentifier: bundleIdentifier,
            queue: .main
        )
        request.delegate = self

        OSSystemExtensionManager.shared.submitRequest(request)

        NotificationCenter.default.post(
            name: ExtensionManager.stateChanged,
            object: NotificationObject(
                bundleIdentifier: bundleIdentifier,
                state: .activationRequested,
                message: "activation of \(bundleIdentifier) is requested"
            )
        )
    }

    func deactivate(_ bundleIdentifier: String) {
        let request = OSSystemExtensionRequest.deactivationRequest(
            forExtensionWithIdentifier: bundleIdentifier,
            queue: .main
        )
        request.delegate = self

        OSSystemExtensionManager.shared.submitRequest(request)

        NotificationCenter.default.post(
            name: ExtensionManager.stateChanged,
            object: NotificationObject(
                bundleIdentifier: bundleIdentifier,
                state: .deactivationRequested,
                message: "deactivation of \(bundleIdentifier) is requested"
            )
        )
    }

    //
    // OSSystemExtensionRequestDelegate
    //

    func request(_ request: OSSystemExtensionRequest,
                 didFinishWithResult result: OSSystemExtensionRequest.Result) {
        NotificationCenter.default.post(
            name: ExtensionManager.stateChanged,
            object: NotificationObject(
                bundleIdentifier: request.identifier,
                state: .requestFinished,
                message: "request of \(request.identifier) is finished"
            )
        )

        switch result {
        case .completed:
            break
        case .willCompleteAfterReboot:
            NotificationCenter.default.post(
                name: ExtensionManager.stateChanged,
                object: NotificationObject(
                    bundleIdentifier: request.identifier,
                    state: .rebootRequired,
                    message: "request of \(request.identifier) requires reboot"
                )
            )
        @unknown default:
            break
        }
    }

    func request(_ request: OSSystemExtensionRequest,
                 didFailWithError error: Error) {
        NotificationCenter.default.post(
            name: ExtensionManager.stateChanged,
            object: NotificationObject(
                bundleIdentifier: request.identifier,
                state: .requestFailed,
                message: "request of \(request.identifier) is failed with error: \(error.localizedDescription)"
            )
        )
    }

    func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {
        NotificationCenter.default.post(
            name: ExtensionManager.stateChanged,
            object: NotificationObject(
                bundleIdentifier: request.identifier,
                state: .userApprovalRequired,
                message: "request of \(request.identifier) requires user approval"
            )
        )
    }

    func request(_ request: OSSystemExtensionRequest,
                 actionForReplacingExtension existing: OSSystemExtensionProperties,
                 withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction {
        let existingVersion = existing.bundleVersion
        let extVersion = ext.bundleVersion

        if extVersion.compare(existingVersion, options: .numeric) == .orderedAscending {
            NotificationCenter.default.post(
                name: ExtensionManager.stateChanged,
                object: NotificationObject(
                    bundleIdentifier: request.identifier,
                    state: .willReplace,
                    message: "\(request.identifier) will replace to \(extVersion) from \(existingVersion)"
                )
            )

            return .replace
        }

        NotificationCenter.default.post(
            name: ExtensionManager.stateChanged,
            object: NotificationObject(
                bundleIdentifier: request.identifier,
                state: .replacingCanceled,
                message: "request of \(request.identifier) is canceled"
            )
        )

        return .cancel
    }
}
