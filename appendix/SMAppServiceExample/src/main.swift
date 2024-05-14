import Foundation
import ServiceManagement

let daemonPlistName = "org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient7.plist"

RunLoop.main.perform {
  for argument in CommandLine.arguments {
    if argument == "register" {
      let s = SMAppService.daemon(plistName: daemonPlistName)
      do {
        try s.register()
        print("Successfully registered \(s)")
      } catch {
        print("Unable to register \(error)")
      }
      return

    } else if argument == "unregister" {
      let s = SMAppService.daemon(plistName: daemonPlistName)
      do {
        try s.unregister()
        print("Successfully unregistered \(s)")
      } catch {
        print("Unable to unregister \(error)")
      }
      return

    } else if argument == "status" {
      let s = SMAppService.daemon(plistName: daemonPlistName)
      switch s.status {
      case .notRegistered:
        print("notRegistered")
      case .enabled:
        print("enabled")
      case .requiresApproval:
        print("requiresApproval")
      case .notFound:
        print("notFound")
      @unknown default:
        print("unknown \(s.status)")
      }
      return
    }
  }

  print("Usage:")
  print("    SMAppServiceExample register|unregister|status")
  exit(0)
}
