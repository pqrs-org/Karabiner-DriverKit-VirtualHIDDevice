import Cocoa
import SwiftUI

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    var window: NSWindow!
    var deviceManager: DeviceManager?

    func applicationDidFinishLaunching(_: Notification) {
        deviceManager = DeviceManager()

        window = NSWindow(
            contentRect: .zero,
            styleMask: [.titled,
                        .closable,
                        .miniaturizable,
                        .resizable,
                        .fullSizeContentView],
            backing: .buffered,
            defer: false
        )
        window.isReleasedWhenClosed = false
        window.contentView = NSHostingView(rootView: ContentView())

        window.center()
        window.makeKeyAndOrderFront(nil)
    }

    public func applicationShouldTerminateAfterLastWindowClosed(_: NSApplication) -> Bool {
        return true
    }
}
