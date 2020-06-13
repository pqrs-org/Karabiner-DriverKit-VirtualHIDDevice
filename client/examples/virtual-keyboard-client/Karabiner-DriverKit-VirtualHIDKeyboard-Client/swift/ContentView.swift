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
                                Text("initialize")
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

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDKeyboardTerminate() }) {
                                Text("terminate")
                            }
                        }

                        Spacer()
                    }.padding()
                }.frame(width: 200.0)

                GroupBox(label: Text("VirtualHIDPointing")) {
                    HStack {
                        VStack(alignment: .leading) {
                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingInitialize() }) {
                                Text("initialize")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingPostExampleReport() }) {
                                Text("post example report")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingReset() }) {
                                Text("reset")
                            }

                            Button(action: { VirtualHIDDeviceClientExample.shared.virtualHIDPointingTerminate() }) {
                                Text("terminate")
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
