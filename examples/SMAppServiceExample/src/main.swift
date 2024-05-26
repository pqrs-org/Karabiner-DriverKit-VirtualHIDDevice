import Foundation
import ServiceManagement

enum Subcommand: String {
  case register
  case unregister
  case status
}

func registerService(_ service: SMAppService) {
  // Regarding daemons, performing the following steps causes inconsistencies in the user approval database,
  // so the process will not start again until it is unregistered and then registered again.
  //
  // 1. Register a daemon.
  // 2. Approve the daemon.
  // 3. The database is reset using `sfltool resetbtm`.
  // 4. Restart macOS.
  //
  // When this happens, the service status becomes .notFound.
  // So, if the service status is .notFound, we call unregister before register to avoid this issue.
  //
  // Another case where it becomes .notFound is when it has never actually been registered before.
  // Even in this case, calling unregister will not have any negative impact.

  if service.status == .notFound {
    unregisterService(service)
  }

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
          // A service that was once registered but then unregistered becomes notRegistered.
          print("\(s) notRegistered")
        case .enabled:
          print("\(s) enabled")
        case .requiresApproval:
          print("\(s) requiresApproval")
        case .notFound:
          // A service that has never been registered becomes notFound. Resetting the database with `sfltool resetbtm` also results in notFound.
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
