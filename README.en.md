# VTMBLELibrary Demo

[中文](README.md) | **English**

The official sample project for Viatom's iOS Bluetooth SDK for medical devices.

This repository holds a ready-to-run sample app plus a prebuilt
`VTMBLELibrary.xcframework`. The SDK source is not in this repository.

> This is a translation of [README.md](README.md). The Chinese text is the
> original; if the two ever disagree, the Chinese wins.

Currently supported devices:

| Device | Description |
| --- | --- |
| WBP02 | Ambulatory blood pressure monitor |
| PM10 | Handheld ECG recorder |
| JMRBP | Bluetooth blood pressure monitor |

---

## Getting started

You need Xcode 15 or later and a **physical device** running iOS 13.0+.

```bash
git clone https://github.com/viatom-dev/VTMBLELibraryDemo.git
cd VTMBLELibraryDemo
pod install
open VTMBLEDemo.xcworkspace
```

Select the `VTMBLEDemo` scheme, pick a real device, and run.

> **It will not work on the simulator.** CoreBluetooth has no Bluetooth hardware
> there: the app launches but the scan screen shows "BLE not supported on this
> device". The sample code compiles for the simulator, but connecting to a device
> requires real hardware.

If you would rather not use CocoaPods, drag
`VTMBLELibrary/VTMBLELibrary.xcframework` straight into your own project; see
"Integrating into your app" below.

---

## Using the sample app

1. Pick the device model at the top (WBP02 / PM10 / JMRBP) — this decides which
   session class gets created
2. Optionally type a device name fragment into the text field to filter
3. Tap "Start scanning", then tap a device in the list to connect
4. Once connected, the app moves to that device's command screen
5. The top half of the command screen is a tappable command list; the bottom half
   shows the result of every call as it happens
6. The "Log" screen in the top-right corner lets you change the SDK log level,
   turn redaction on, and export a log file

For WBP02, tap "Handshake" first; every other command depends on it. PM10 and
JMRBP need no handshake.

The fifth group on the JMRBP screen, "To be verified", exists for hardware
verification: each of the points the protocol document leaves unclear (the order
records arrive in, whether a device acknowledgement is equivalent to "marked as
uploaded", whether an interrupted transfer resumes) has a button that prints its
verdict in the bottom half. Set the log level to `Debug` and watch both at once.

---

## Code tour

Read in this order to cover everything a full integration needs:

| File | What it demonstrates |
| --- | --- |
| `VTMDemoCentralManager.h/.m` | **Scanning and connecting. This is not the SDK's job** and you have to implement it yourself. The place integrations get stuck most often. |
| `VTMDemoDeviceViewController.m` | The full session lifecycle: create → attach peripheral → wait for deployment → issue commands → detach callbacks when the screen goes away |
| `VTMDemoWBP02ViewController.m` | Every WBP02 public API, including the handshake chain, programming writes, and device-initiated live data |
| `VTMDemoPM10ViewController.m` | Every PM10 public API, including paged case-list fetching, waveform download and the progress callback |
| `VTMDemoJMRBPViewController.m` | Every JMRBP public API. This device has no handshake and almost no request/response pairing, so it is the reference for the "one-way commands plus device-initiated reports" shape; it also shows the idempotent full sync for stored records |
| `VTMDemoLogViewController.m` | How to use `VTMBLELogger`: levels, redaction, live subscription, file export |
| `AppDelegate.m` | Configuring SDK logging at launch |

---

## Integrating into your app

### CocoaPods

Copy the `VTMBLELibrary/` directory (which contains both the `.podspec` and the
`.xcframework`) into your project:

```ruby
pod 'VTMBLELibrary', :path => 'VTMBLELibrary'
```

### Swift Package Manager

In Xcode, File > Add Package Dependencies, and enter:

```
https://github.com/viatom-dev/VTMBLELibraryDemo.git
```

### Manual

Drag `VTMBLELibrary.xcframework` into your project and set it to **Do Not Embed**
under your target's General > Frameworks, Libraries, and Embedded Content (it is a
static framework and does not need embedding).

You also need `NSBluetoothAlwaysUsageDescription` in your Info.plist, or the
system terminates the app the moment it asks for Bluetooth permission.

---

## Minimal integration

The SDK only covers "the protocol layer above an already connected peripheral".
Scanning, connecting and reconnecting belong to your `CBCentralManager`. The full
sequence:

```objc
#import <VTMBLELibrary/VTMBLELibrary.h>

// 1. Scan and connect with your own CBCentralManager (the SDK does not do this)

// 2. Create the session. Note that -init / +new on VTMBLECoreSession are
//    NS_UNAVAILABLE; you must go through the device class's +session
self.session = [VTMWBP02BLESession session];

// 3. Set the delegate
self.session.sessionDelegate = self;

// 4. Hand the connected peripheral to the session.
//    ⚠️ This assignment starts service and characteristic discovery — it is a
//    setter with side effects
self.session.peripheral = connectedPeripheral;

// 5. Wait for this callback. Commands issued before it arrives are lost, because
//    the characteristic used for writing has not been found yet
- (void)sessionDeployCompletion:(VTMBLECoreSession *)session {
    // 6. Now commands may be sent
    [self.session handshakes:^(NSString *deviceSN) {
        NSLog(@"SN = %@", deviceSN);
    }];
}
```

---

## Key constraints (please read all of them)

### You must wait for `-sessionDeployCompletion:`

The SDK only starts discovering services after `session.peripheral = ...`. Any
command method called before deployment finishes fails silently.

### Commands must be issued one at a time

The SDK keeps a command queue internally, but **do not call concurrently**: wait
for the previous callback before sending the next command. The sample app's
command list is built around this.

### Threading

The SDK does not lock its command queue or receive buffer. Keep "calling SDK
APIs" and "CoreBluetooth callbacks" on one and the same queue.

The simplest approach is to put everything on the main thread: pass
`dispatch_get_main_queue()` when creating `CBCentralManager`, which is what the
sample code does. SDK callbacks run on the CoreBluetooth callback queue, i.e. the
queue you handed to the central manager.

### Callback arguments can be nil

The headers are wrapped in `NS_ASSUME_NONNULL`, yet some callbacks do pass nil.
WBP02's live data is the classic case:

```objc
[session receiveRealtimeData:^(NSNumber *pressure, VTMWBP02DataModel *result) {
    // while inflating:      (pressure, nil)
    // when measurement ends: (nil, result model)
    if (pressure != nil) { /* ... */ }
    if (result   != nil) { /* ... */ }
}];
```

**Swift callers especially take note**: these arguments are imported as
non-optional types, so receiving nil crashes outright. This will be fixed in the
next major version with a unified signature carrying an `error`.

### Device-initiated report APIs only register; they send nothing

Methods like `-receiveRealtimeData:` and `-monitorPressureEnd:` merely store the
block; nothing is sent to the device. Register them as early as possible after
deployment finishes, or nothing will be there to receive what the device pushes.

### Logging and patient privacy

SDK logging is fully disabled by default. Raw frames contain patient
physiological data, so **keep `VTMBLELogLevelOff` in production**:

```objc
// Turn it on while diagnosing a problem
VTMBLELogger.sharedLogger.level = VTMBLELogLevelDebug;

// To watch traffic shape in production, keep only the routing bytes so that
// measurement values are not recorded
VTMBLELogger.sharedLogger.redactsPayload = YES;

// Export it for us after a problem occurs
NSURL *dir = [NSFileManager.defaultManager URLsForDirectory:NSCachesDirectory
                                                 inDomains:NSUserDomainMask].firstObject;
NSError *error = nil;
NSURL *logFile = [VTMBLELogger.sharedLogger writeLogFileToDirectory:dir error:&error];
```

The `handler` callback runs on the logger's internal serial queue. Do not call any
VTMBLELibrary method from inside it.

---

## Known limitations

Listed honestly so you do not walk into them:

- **There is no timeout.** When a device does not return the expected response,
  that command's callback never fires and the queue stalls every command behind
  it. Add your own timeout at the business layer.
- **There is no unified error callback.** Every existing callback takes a single
  argument, so failures such as failed service discovery or a failed write are
  never reported to the caller — they only reach the SDK log.
- **`peripheral` is a setter with side effects**, not a plain storage property.
- **Nullability annotations do not match actual behavior**; see "Callback
  arguments can be nil" above.
- **PM10 waveform decoding belongs to the caller.**
  `-requestCaseDataWithInfo:progressHandle:callback:` calls back with raw
  `NSData`; use `VTMPM10CaseDataMdoel` to turn it into a µV sequence. On the
  result, `infoModel` is **never assigned by the SDK and is always `nil`** — fill
  it in yourself if you need the association.
- **Use `VTMPM10ReqCaseInfoAll` for PM10 case lists.** The device only reports a
  total count and an un-uploaded count, and the SDK uses the total as its
  "everything arrived" criterion. So `Uploaded` may never reach it and never call
  back, and `Target` does not fetch "only that one entry" but keeps walking
  forward from that index. Filter by `uploadState` / `serialNumber` on your side
  once you have the list.
- **The verdict for PM10's last eight `supportLanguages` entries is
  unconfirmed.** Of the three language bitmask groups, the second and third read
  the same byte, which shows up as `PL`~`DE` and `JP`~`NL` always appearing or
  disappearing together. Do not rely on them for now.
- **JMRBP stored-record reads are not guaranteed to be complete in one pass.**
  The protocol gives neither a total count nor an end marker, so the SDK can only
  close on a silence timeout and `Idle` does not mean "everything arrived". Fetch
  unconditionally on every connection and merge by slot + timestamp; do not keep a
  sync cursor — if an interrupted sync advances the cursor, earlier records are
  skipped permanently.
- **JMRBP's start / stop commands have no callback.** The protocol defines no
  response for them, so the SDK offers none (no callback is invented for a
  response that does not exist). Watch the pressure reports to tell whether the
  device actually started.
- **A JMRBP acknowledgement (ACK) tells the device "you may forget this one".**
  Once acknowledged, the device marks the record as uploaded: it neither resends
  it nor converts it into a stored record that could be read again — that data is
  gone. If you persist inside the callback and persisting can fail, set
  `acknowledgesMeasurementResult` to `NO` and call
  `-acknowledgeMeasurementResult` after a successful write. The fourth group on
  the demo's JMRBP screen toggles this directly.
- **JMRBP stored records cannot currently be read.** The sample unit returns
  nothing at all for the read command (the vendor's own demo cannot read them
  either). The command is suspected to have a precondition the protocol document
  does not describe; this is being confirmed with the vendor.
- **A few JMRBP bytes still need hardware verification**, among them the pressure
  conversion factor and the four `reserved` bytes. They are exposed as raw codes
  for now.
- **Conclusion wordings are only built in for ZH / EN**, while the PM10 device
  itself supports 18 languages. To cover more, take the raw values from
  `resultCodes` and localize them yourself.
- Some already published API names are misspelled (`getBatteyInfo:`,
  `daylightEntTime`, `VTMPM10CaseDataMdoel`, `initWitData:`). They will not be
  renamed, so that existing integrations keep compiling.

---

## For maintainers

The following requires access to the private source repository. Third-party
integrators can skip it.

### Demo code changes only

Commit and push as usual; the binary does not need touching.

### SDK source changes

```bash
# 1. Change the code in the private repo and debug it in the demo in source mode
cd VTMBLELibraryDemo
VTM_BLE_SDK_SOURCE=../path/to/vtmblelibrary pod install

# 2. When done, commit in the private repo and push to the internal network

# 3. Sync into this repository: rebuild → copy → switch back to binary mode → verify build
cd ../path/to/vtmblelibrary
./Scripts/sync-demo.sh

# 4. Commit as instructed at the end of the script
```

**You must compile once in binary mode before releasing.** Two classes of
problem — a public header missing from the manifest, and a public header
importing a private header — never surface in source mode. `sync-demo.sh` forces
that step.

### Releasing

The version number is written in four places, and maintaining it by hand
guarantees drift. Run the consistency check before releasing:

```bash
cd /path/to/vtmblelibrary
./Scripts/check-versions.sh
```

It verifies `MARKETING_VERSION` in the private repo (Debug and Release must
match), the source podspec, this repository's binary podspec, and the latest
released version in the CHANGELOG. `sync-demo.sh` runs it first and aborts on a
mismatch.

Note that `MARKETING_VERSION` is baked into the framework's `Info.plist`, so
changing the version requires repackaging, or integrators will read a different
version than you expect.

Once the version is final, tag both repositories with the same name (always with
the `v` prefix):

```bash
git tag -a v1.0.0 -m "..."   # in the private repo and in this one
git push origin v1.0.0
```

### Local commit guard

The demo repo installs a `pre-commit` hook that catches two "works locally,
breaks once public" mistakes: a source-mode `Podfile.lock` (it contains a path
outside the repository, so third-party `pod install` fails), and private headers
finding their way into the XCFramework.

The hook lives under `.git/hooks/` and is not distributed with the repository.
Reinstall it after switching machines or re-cloning:

```bash
./Scripts/install-demo-hooks.sh /path/to/VTMBLELibraryDemo
```

---

## Version

Currently `1.0.0`, tagged `v1.0.0`. Supports `ios-arm64` and
`ios-arm64_x86_64-simulator`.

The version is embedded in the binary too, so you can confirm which build you
have when reporting a problem:

```bash
/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' \
  VTMBLELibrary/VTMBLELibrary.xcframework/ios-arm64/VTMBLELibrary.framework/Info.plist
```

Versioning follows [Semantic Versioning](https://semver.org/). A breaking change
to the public API bumps major, and the "Known limitations" section here is updated
in step.

## License

The sample app source is released under the MIT license.
`VTMBLELibrary.xcframework` is proprietary Viatom software; see
[LICENSE](LICENSE) for its terms.

## Contact

For technical questions please open an issue or email ios@viatomtech.com. Include
the exported SDK log file, the device model and the SDK version.
