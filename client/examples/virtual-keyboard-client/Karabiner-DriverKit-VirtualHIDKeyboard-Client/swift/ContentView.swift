import SwiftUI

struct ContentView: View {
    let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as! String

    var body: some View {
        VStack {
            Text("Karabiner-DriverKit-VirtualHIDKeyboard-Client version " + self.version)

            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardInitialize() }) {
                Text("VirtualHIDKeyboard initialize")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardPostControlUp() }) {
                Text("post control-up")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardPostLaunchpad() }) {
                Text("post launchpad")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardReset() }) {
                Text("reset")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingInitialize() }) {
                Text("VirtualHIDPointing initialize")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingPostExampleReport() }) {
                Text("VirtualHIDPointing post example report")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingReset() }) {
                Text("VirtualHIDPointing reset")
            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity).padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
