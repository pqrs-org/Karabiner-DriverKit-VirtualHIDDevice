try
    display dialog "Are you sure you want to remove Karabiner-DriverKit-VirtualHIDDevice?" buttons {"Cancel", "OK"}
    if the button returned of the result is "OK" then
        try
            do shell script "test -f '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts/uninstall.sh'"
            try
                do shell script "bash '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts/uninstall.sh'" with administrator privileges
            on error
                display alert "Failed to uninstall Karabiner-DriverKit-VirtualHIDDevice."
            end try
        on error
            display alert "Karabiner-DriverKit-VirtualHIDDevice is not installed."
        end try
    end if
on error
    display alert "Karabiner-DriverKit-VirtualHIDDevice uninstallation was canceled."
end try
