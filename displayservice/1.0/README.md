# Legacy displayservice client library

The 32-bit X00TD `camera.sdm660.so` links against
`android.frameworks.displayservice@1.0.so`. Android 17 removed this interface
from its source tree and HIDL module allowlist, so this directory builds the
frozen generated transport as a shared library, including its vendor variant.

The four HAL definitions are unchanged from AOSP `android-10.0.0_r1`.
The C++ files were generated with AOSP `android-17.0.0_r1`'s `hidl-gen`
(SHA-256 `9a3101a5a1c5db874ad661573cb60b324cea328954a10c9c750dcf3b6bd96863`).
Both generated directories can be reproduced from the Android source root:

```sh
for entry in 'c++-sources generated' 'c++-headers include'; do
    set -- $entry
    LD_LIBRARY_PATH=prebuilts/build-tools/linux-x86/lib64 \
        prebuilts/build-tools/linux-x86/bin/hidl-gen -R -p . \
        -o device/asus/sdm660-common/displayservice/1.0/"$2" -L "$1" \
        -randroid.hidl:system/libhidl/transport \
        -randroid.frameworks.displayservice:device/asus/sdm660-common/displayservice \
        android.frameworks.displayservice@1.0
done
```

This restores the client transport, without registering a displayservice
backend. The camera blob checks for a null service in `QCameraDisplay::init`;
camera operation and vsync behavior still require validation on the device.
