import ServiceManagement

let daemonPlistName = "org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist"

@_cdecl("service_manager_register")
func service_manager_register() {
  let daemon = SMAppService.daemon(plistName: daemonPlistName)

  do {
    try daemon.register()
    print("registered")
  } catch {
    print("Unable to register \(error)")
  }
}

@_cdecl("service_manager_unregister")
func service_manager_unregister() {
  let daemon = SMAppService.daemon(plistName: daemonPlistName)

  do {
    try daemon.unregister()
    print("unregistered")
  } catch {
    print("Unable to register \(error)")
  }
}
