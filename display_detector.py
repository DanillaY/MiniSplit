import os
import platform
import subprocess
from enum import Enum

class DisplayBackend(Enum):
    WINDOWS = "windows"
    X11 = "x11"
    XWAYLAND = "xwayland"
    WAYLAND = "wayland"
    UNKNOWN = "unknown"

display: DisplayBackend = DisplayBackend.UNKNOWN

def detect_display_backend():
    global display

    if platform.system().lower() == "windows":
        display = DisplayBackend.WINDOWS
        return DisplayBackend.WINDOWS

    wayland = bool(os.environ.get("WAYLAND_DISPLAY"))
    x11 = bool(os.environ.get("DISPLAY"))

    if x11 & wayland == False:
        display = DisplayBackend.X11
        return DisplayBackend.X11

    if wayland:
        try:
            proc = subprocess.run(
                ["xprop", "-root"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            if proc.returncode == 0:
                display = DisplayBackend.XWAYLAND
                return DisplayBackend.XWAYLAND

            display = DisplayBackend.WAYLAND
            return DisplayBackend.WAYLAND
            
        except FileNotFoundError:
            display = DisplayBackend.WAYLAND
            return DisplayBackend.WAYLAND

    return DisplayBackend.UNKNOWN
