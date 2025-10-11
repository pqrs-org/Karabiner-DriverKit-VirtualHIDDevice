import AppKit
import Foundation

@main
struct Manager {
  static func main() async {
    do {
      let args = CommandLine.arguments.dropFirst()

      if args.contains("activate") {
        _ = try await ExtensionManager.perform(.activate)
        exit(0)
      } else if args.contains("forceActivate") {
        _ = try await ExtensionManager.perform(.forceActivate)
        exit(0)
      } else if args.contains("deactivate") {
        _ = try await ExtensionManager.perform(.deactivate)
        exit(0)
      } else {
        print("Usage:")
        print("    Karabiner-VirtualHIDDevice-Manager activate|forceActivate|deactivate")
        print("")

        //
        // Open Karabiner-Elements Settings
        //

        // If `/Applications/.Karabiner-VirtualHIDDevice-Manager.app` shows up in Spotlight's Applications list
        // and gets run, doing nothing would confuse users, so open the Karabiner-Elements settings.

        if args.isEmpty {
          let url = URL(fileURLWithPath: "/Applications/Karabiner-Elements.app")
          if FileManager.default.fileExists(atPath: url.path) {
            print("Opening \(url)...")

            let configuration = NSWorkspace.OpenConfiguration()
            let _: NSRunningApplication? = try await withCheckedThrowingContinuation { continuation in
              NSWorkspace.shared.openApplication(at: url, configuration: configuration) { runningApp, error in
                if let error {
                  continuation.resume(throwing: error)
                } else {
                  continuation.resume(returning: runningApp)
                }
              }
            }
          }
        }

        exit(0)
      }
    } catch {
      fputs("Error: \(error)\n", stderr)
      exit(1)
    }
  }
}
