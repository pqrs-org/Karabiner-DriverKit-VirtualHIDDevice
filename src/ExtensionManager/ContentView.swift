import SwiftUI
import SystemExtensions

struct ContentView: View {
    private let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as! String
    private let driverVersion = Bundle.main.object(forInfoDictionaryKey: "KarabinerDriverKitVirtualHIDDeviceDriverVersion") as! String
    private let driverIdentifier = "org.pqrs.Karabiner-DriverKit-VirtualHIDDevice"

    @State private var logMessages: [String] = []
    @State private var controlResult: String = ""

    func runList() {
        controlResult = "systemextensionsctl list\n\n" + ExtensionManager.shared.list()
    }

    func runReset() {
        controlResult = "systemextensionsctl reset\n\n" + ExtensionManager.shared.reset()
    }

    var body: some View {
        VStack {
            Text("Karabiner-DriverKit-ExtensionManager version " + self.version)
            Text("Karabiner-DriverKit-VirtualHIDDevice version " + self.driverVersion)
            HStack {
                Button(action: { ExtensionManager.shared.activate(self.driverIdentifier) }) {
                    Text("Activate driver extension")
                }
                Button(action: { ExtensionManager.shared.deactivate(self.driverIdentifier) }) {
                    Text("Deactivate driver extension")
                }
            }
            ScrollView {
                VStack(alignment: .leading, spacing: 10) {
                    ForEach(logMessages, id: \.self) { logMessage in
                        Text(logMessage)
                    }
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
                .padding()
            }
            .overlay(RoundedRectangle(cornerRadius: 5).stroke(lineWidth: 1))

            HStack {
                Button(action: { self.runList() }) {
                    Text("systemextensionsctl list")
                }
                Button(action: { self.runReset() }) {
                    Text("systemextensionsctl reset")
                }
            }
            ScrollView {
                VStack(alignment: .leading, spacing: 10) {
                    Text(controlResult)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
                .padding()
            }
            .overlay(RoundedRectangle(cornerRadius: 5).stroke(lineWidth: 1))
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity).padding()
        .onReceive(NotificationCenter.default.publisher(for: ExtensionManager.stateChanged)) { obj in
            self.logMessages.append((obj.object as! ExtensionManager.NotificationObject).message)
            self.runList()
        }
        .onAppear { self.runList() }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
