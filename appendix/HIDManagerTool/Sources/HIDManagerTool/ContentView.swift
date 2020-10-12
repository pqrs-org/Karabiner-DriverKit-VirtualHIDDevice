import SwiftUI

struct ContentView: View {
    @ObservedObject var deviceManager = DeviceManager.shared

    var body: some View {
        VStack {
            Text($deviceManager.virtualHIDKeyboard.wrappedValue != nil ? "VirtualHIDKeyboard is ready" : "VirtualHIDKeyboard is not ready")
                .fixedSize()

            Button(action: { DeviceManager.shared.setHIDValue(1) }) {
                Text("Set HIDValue 1")
            }

            Button(action: { DeviceManager.shared.setHIDValue(1) }) {
                Text("Set HIDValue 0")
            }
        }.padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
