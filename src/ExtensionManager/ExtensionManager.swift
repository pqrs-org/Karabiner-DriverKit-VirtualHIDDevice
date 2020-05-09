import Foundation
import os.log
import SystemExtensions

class ExtensionManager: NSObject, OSSystemExtensionRequestDelegate {
    static let shared = ExtensionManager()
    let driverIdentifier = "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard"

    func activate() {
        os_log("%@ activate", driverIdentifier)

        let request = OSSystemExtensionRequest.activationRequest(
            forExtensionWithIdentifier: driverIdentifier,
            queue: .main
        )
        request.delegate = self

        OSSystemExtensionManager.shared.submitRequest(request)
    }

    func deactivate() {
        os_log("%@ deactivate", driverIdentifier)

        let request = OSSystemExtensionRequest.deactivationRequest(
            forExtensionWithIdentifier: driverIdentifier,
            queue: .main
        )
        request.delegate = self

        OSSystemExtensionManager.shared.submitRequest(request)
    }

    //
    // OSSystemExtensionRequestDelegate
    //

    func request(_: OSSystemExtensionRequest,
                 didFinishWithResult result: OSSystemExtensionRequest.Result) {
        os_log("ExtensionManager request didFinishWithResult:%@", result.rawValue)
    }

    func request(_: OSSystemExtensionRequest,
                 didFailWithError error: Error) {
        os_log("ExtensionManager request didFailWithError:%@", error.localizedDescription)
    }

    func requestNeedsUserApproval(_: OSSystemExtensionRequest) {
        os_log("ExtensionManager requestNeedsUserApproval")
    }

    func request(_: OSSystemExtensionRequest,
                 actionForReplacingExtension existing: OSSystemExtensionProperties,
                 withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction {
        os_log("ExtensionManager request actionForReplacingExtension:%@ withExtension:%@", existing, ext)

        return .replace
    }
}
