import ServiceManagement

let daemonPlistName = "org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist"

@_cdecl("service_manager_register")
func service_manager_register() -> Bool {
  let s = SMAppService.daemon(plistName: daemonPlistName)

  do {
    try s.register()
    print("Successfully registered \(s)")
    return true
  } catch {
    print("Unable to register \(error)")
    return false
  }
}

@_cdecl("service_manager_unregister")
func service_manager_unregister() -> Bool {
  let s = SMAppService.daemon(plistName: daemonPlistName)

  do {
    try s.unregister()
    print("Successfully unregistered \(s)")
    return true
  } catch {
    print("Unable to unregister \(error)")
    return false
  }
}

@_cdecl("service_manager_status")
func service_manager_status() {
  let daemon = SMAppService.daemon(plistName: daemonPlistName)

  switch daemon.status {
  case .notRegistered:
    print("notRegistered")
  case .enabled:
    print("enabled")
  case .requiresApproval:
    print("requiresApproval")
  case .notFound:
    print("notFound")
  @unknown default:
    print("unknown \(daemon.status)")
  }
}
