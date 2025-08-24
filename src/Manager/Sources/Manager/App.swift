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
        exit(0)
      }
    } catch {
      fputs("Error: \(error)\n", stderr)
      exit(1)
    }
  }
}
