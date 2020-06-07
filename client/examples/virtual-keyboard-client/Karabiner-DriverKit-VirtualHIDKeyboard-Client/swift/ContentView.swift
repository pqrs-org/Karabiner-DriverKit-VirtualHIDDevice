import SwiftUI

struct ContentView: View {
    let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as! String

    var body: some View {
        VStack {
            Text("Karabiner-DriverKit-VirtualHIDKeyboard-Client version " + self.version)

            Button(action: { VirtualHIDDeviceClientExample.shared.postControlUp() }) {
                Text("post control-up")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.postLaunchpad() }) {
                Text("post launchpad")
            }

            Button(action: { VirtualHIDDeviceClientExample.shared.reset() }) {
                Text("reset")
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
