import Foundation
import SystemExtensions

class ExtensionManager: NSObject, OSSystemExtensionRequestDelegate {
    enum Mode {
        case none
        case activation
        case deactivation
    }

    static let shared = ExtensionManager()

    let bundleIdentifier = "org.pqrs.Karabiner-DriverKit-VirtualHIDDevice"
    var forceReplace = false
    var mode = Mode.none

    //
    // macOS Catalina (10.15) does not support driver replacing.
    //
    // Replacing running driver on macOS Catalina, the new driver will not be loaded properly even if restarting macOS.
    // If you replaced the running driver, you have to restart macOS, deactivate new driver, restart macOS, and then activate it again.
    //
    // It is more easier way to refuse replacing and force deactivating driver, restarting macOS, and activating new driver.
    //

    let replaceSupported = ProcessInfo().isOperatingSystemAtLeast(OperatingSystemVersion(majorVersion: 11, minorVersion: 0, patchVersion: 0))

    //
    // Actions
    //

    func activate(forceReplace: Bool) {
        mode = Mode.activation
        self.forceReplace = forceReplace

        let request = OSSystemExtensionRequest.activationRequest(
            forExtensionWithIdentifier: bundleIdentifier,
            queue: .main
        )
        request.delegate = self

        OSSystemExtensionManager.shared.submitRequest(request)

        print("activation of \(bundleIdentifier) is requested")
    }

    func deactivate() {
        mode = Mode.deactivation

        let request = OSSystemExtensionRequest.deactivationRequest(
            forExtensionWithIdentifier: bundleIdentifier,
            queue: .main
        )
        request.delegate = self

        OSSystemExtensionManager.shared.submitRequest(request)

        print("deactivation of \(bundleIdentifier) is requested")
    }

    //
    // OSSystemExtensionRequestDelegate
    //

    func request(_ request: OSSystemExtensionRequest,
                 didFinishWithResult result: OSSystemExtensionRequest.Result)
    {
        print("request of \(request.identifier) is finished")

        switch result {
        case .completed:
            print("request of \(request.identifier) is completed")
        case .willCompleteAfterReboot:
            print("request of \(request.identifier) requires reboot")
            break
        @unknown default:
            break
        }

        exit(0)
    }

    func request(_ request: OSSystemExtensionRequest,
                 didFailWithError error: Error)
    {
        print("request of \(request.identifier) is failed with error: \(error.localizedDescription)")

        switch mode {
        case .none:
            exit(1)
        case .activation:
            exit(1)
        case .deactivation:
            switch error {
            case OSSystemExtensionError.extensionNotFound:
                // Ignorable errors
                exit(0)
            default:
                exit(1)
            }
        }

        exit(1)
    }

    func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {
        print("request of \(request.identifier) requires user approval")
    }

    func request(_ request: OSSystemExtensionRequest,
                 actionForReplacingExtension existing: OSSystemExtensionProperties,
                 withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction
    {
        let existingVersion = existing.bundleVersion
        let extVersion = ext.bundleVersion

        if forceReplace {
            print("\(request.identifier) will be force replaced to \(extVersion) forcely")
            return .replace
        }

        if extVersion.compare(existingVersion, options: .numeric) == .orderedDescending {
            if !replaceSupported {
                print("request of \(request.identifier) is canceled because replacing is not support")
                return .cancel
            }

            print("\(request.identifier) will be replaced to \(extVersion) from \(existingVersion)")
            return .replace
        }

        print("request of \(request.identifier) is canceled because newer version (\(existingVersion)) is already installed")
        return .cancel
    }
}
