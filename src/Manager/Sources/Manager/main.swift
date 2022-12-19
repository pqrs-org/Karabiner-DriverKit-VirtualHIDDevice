import Foundation

RunLoop.main.perform {
  for argument in CommandLine.arguments {
    if argument == "activate" {
      ExtensionManager.shared.activate(forceReplace: false)
      return
    } else if argument == "forceActivate" {
      ExtensionManager.shared.activate(forceReplace: true)
      return
    } else if argument == "deactivate" {
      ExtensionManager.shared.deactivate()
      return
    }
  }

  print("Usage:")
  print("    Karabiner-VirtualHIDDevice-Manager activate|forceActivate|deactivate")
  exit(0)
}

RunLoop.main.run()
