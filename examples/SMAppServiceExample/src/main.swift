import Foundation
import ServiceManagement

enum Subcommand: String {
  case register = "register"
  case unregister = "unregister"
  case status = "status"
}

func registerService(_ service: SMAppService) {
  do {
    try service.register()
    print("Successfully registered \(service)")
  } catch {
    // Note:
    // When calling `SMAppService.daemon.register`, if user approval has not been granted, an `Operation not permitted` error will be returned.
    // To call `register` for all agents and daemons, the loop continues even if an error occurs.
    // Therefore, only log output will be performed here.
    print("Unable to register \(error)")
  }
}

func unregisterService(_ service: SMAppService) {
  do {
    try service.unregister()
    print("Successfully unregistered \(service)")
  } catch {
    print("Unable to unregister \(error)")
  }
}

RunLoop.main.perform {
  let services: [SMAppService] = [
    SMAppService.daemon(
      plistName: "org.pqrs.service.daemon.Karabiner-VirtualHIDDevice-Daemon.plist")
  ]

  if CommandLine.arguments.count > 1 {
    let subcommand = CommandLine.arguments[1]

    switch Subcommand(rawValue: subcommand) {
    case .register:
      for s in services {
        registerService(s)
      }
      exit(0)

    case .unregister:
      for s in services {
        unregisterService(s)
      }
      exit(0)

    case .status:
      for s in services {
        switch s.status {
        case .notRegistered:
          print("\(s) notRegistered")
        case .enabled:
          print("\(s) enabled")
        case .requiresApproval:
          print("\(s) requiresApproval")
        case .notFound:
          print("\(s) notFound")
        @unknown default:
          print("\(s) unknown \(s.status)")
        }
      }

      exit(0)

    default:
      print("Unknown subcommand \(subcommand)")
      exit(1)
    }
  }

  print("Usage:")
  print("    SMAppServiceExample subcommand")
  print("")
  print("Subcommands:")
  print("    \(Subcommand.register.rawValue)")
  print("    \(Subcommand.unregister.rawValue)")
  print("    \(Subcommand.status.rawValue)")

  exit(0)
}

RunLoop.main.run()
