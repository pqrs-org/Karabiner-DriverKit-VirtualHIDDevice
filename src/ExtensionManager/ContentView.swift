import SwiftUI
import SystemExtensions

struct ContentView: View {
    let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as! String

    var body: some View {
        VStack {
            Text("KarabinerDriverKitVirtualHIDDevice version " + self.version)
            HStack {
                Button(action: ExtensionManager.shared.activate) {
                    Text("Activate")
                }
                Button(action: ExtensionManager.shared.deactivate) {
                    Text("Deactivate")
                }
            }
        }.frame(maxWidth: .infinity, maxHeight: .infinity)
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
