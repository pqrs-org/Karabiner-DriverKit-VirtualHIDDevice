import SwiftUI

struct ContentView: View {
    let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as! String

    var body: some View {
        VStack(spacing: 20) {
            Text("Karabiner-DriverKit-VirtualHIDKeyboard-Client version " + self.version)

            HStack(alignment: .top, spacing: 20.0) {
                GroupBox(label: Text("VirtualHIDKeyboard")) {
                    HStack {
                        VStack(alignment: .leading) {
                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardInitialize() }) {
                                Text("1. initialize")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardPostControlUp() }) {
                                Text("2. post control-up")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardPostLaunchpad() }) {
                                Text("3. post launchpad")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardReset() }) {
                                Text("4. reset")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardTerminate() }) {
                                Text("5. terminate")
                            }
                        }

                        Spacer()
                    }.padding()
                }.frame(width: 200.0)

                GroupBox(label: Text("VirtualHIDPointing")) {
                    HStack {
                        VStack(alignment: .leading) {
                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingInitialize() }) {
                                Text("1. initialize")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingPostExampleReport() }) {
                                Text("2. post example report")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingReset() }) {
                                Text("3. reset")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingTerminate() }) {
                                Text("4. terminate")
                            }
                        }

                        Spacer()
                    }.padding()
                }.frame(width: 200.0)
            }

            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity).padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
