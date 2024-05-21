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