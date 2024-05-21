# RFU_Bluetooth_Plugin
Remote Unlock Bluetooth Plugin
This plugin extends the functionality of RemoteFingerUnlock by enabling Bluetooth unlocking capabilities for the Windows lock screen.

## Usage
With this plugin installed, users can utilize Bluetooth devices to unlock their Windows lock screen through the RemoteFingerUnlock application.

## Installation
Build Method:
To build the plugin, use the following cmake command:

**Build Method**: To build the plugin, use the following cmake command:

```
cmake -G Ninja  -B ./build
cmake --build  ./build --target all
```

**Installation**:

- After building the plugin, use the provided `install.reg` file to install it. Double-clicking on the `install.reg` file will add the necessary registry entries to place the plugin in the appropriate location.

## Compatibility

This plugin is compatible with RemoteFingerUnlock on Windows operating systems.

## Feedback and Support

For any issues or feedback regarding this plugin, please visit the support page on our website.

## Download

You can download RemoteFingerUnlock and related plugins from [rfu.czqu.net](https://rfu.czqu.net/).

## License

This project is licensed under the terms of the GNU General Public License v3.0 (GPLv3). You should have received a copy of the GNU General Public License along with this project. If not, see GPLv3 License.

## Third-Party Libraries

### log.c

I use the `log.c` library developed by rxi in this project. Modifications have been made to support `wchar`.

### Windows Classic Samples

I referenced code from the Windows Classic Samples repository by Microsoft for the Bluetooth implementation in this project. The code is licensed under the MIT License.
