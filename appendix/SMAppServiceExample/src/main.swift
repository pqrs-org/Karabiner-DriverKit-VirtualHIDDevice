import Foundation
import ServiceManagement

let daemonPlistName = "org.pqrs.Karabiner-DriverKit-SMAppServiceExample.plist"

RunLoop.main.perform {
  for argument in CommandLine.arguments {
    if argument == "register" {
      let s = SMAppService.daemon(plistName: daemonPlistName)
      do {
        try s.register()
        print("Successfully registered \(s)")
        exit(0)
      } catch {
        print("Unable to register \(error)")
        exit(1)
      }

    } else if argument == "unregister" {
      let s = SMAppService.daemon(plistName: daemonPlistName)
      do {
        try s.unregister()
        print("Successfully unregistered \(s)")
        exit(0)
      } catch {
        print("Unable to unregister \(error)")
        exit(1)
      }

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

      exit(0)
    }
  }

  print("Usage:")
  print("    SMAppServiceExample register|unregister|status")
  exit(0)
}

RunLoop.main.run()
