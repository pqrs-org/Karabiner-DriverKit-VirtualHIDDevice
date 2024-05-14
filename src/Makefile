all:
	$(MAKE) -C Daemon
	$(MAKE) -C DriverKit
	$(MAKE) -C Manager
	mkdir -p Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app/Contents/Library/SystemExtensions
	rsync -a --delete \
		DriverKit/build/Release-driverkit/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext \
		Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app/Contents/Library/SystemExtensions
	$(MAKE) codesign

clean:
	$(MAKE) -C Daemon clean
	$(MAKE) -C DriverKit clean
	$(MAKE) -C Manager clean

codesign:
	bash scripts/codesign.sh

log-show:
	log show --predicate 'sender == "sysextd" or sender CONTAINS "org.pqrs"' --info --debug --last 1h
