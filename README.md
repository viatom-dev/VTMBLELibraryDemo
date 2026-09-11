# VTMBLELibrary Demo

**中文** | [English](README.en.md)

Viatom 医疗设备 iOS 蓝牙 SDK 的官方示例工程。

本仓库包含一个可直接运行的示例 App，以及编译好的 `VTMBLELibrary.xcframework`。
SDK 源码不在本仓库内。

当前支持的设备：

| 设备 | 说明 |
| --- | --- |
| WBP02 | 动态血压计 |
| PM10 | 手持心电 |
| JMRBP | 蓝牙血压计 |

---

## 快速开始

需要 Xcode 15 或更高版本、iOS 13.0+ 的**真机**。

```bash
git clone https://github.com/viatom-dev/VTMBLELibraryDemo.git
cd VTMBLELibraryDemo
pod install
open VTMBLEDemo.xcworkspace
```

选中 `VTMBLEDemo` scheme，选一台真机，运行。

> **模拟器跑不了。** CoreBluetooth 在模拟器上没有蓝牙硬件，App 能启动，
> 扫描页会显示「本设备不支持 BLE」。示例代码可以在模拟器上编译，
> 但要连设备必须用真机。

不想用 CocoaPods 也可以：`VTMBLELibrary/VTMBLELibrary.xcframework` 直接拖进
你自己的工程即可，见下面的「集成到你的 App」。

---

## 示例 App 怎么用

1. 顶部选设备型号（WBP02 / PM10 / JMRBP）—— 决定创建哪个 session 类
2. 需要的话在输入框里填设备名关键字过滤
3. 点「开始扫描」，从列表里选一台设备，点一下即开始连接
4. 连上后自动进入该设备的指令页
5. 指令页上半屏是可点的指令列表，下半屏实时显示每次调用的结果
6. 右上角「日志」页可以调 SDK 日志等级、开脱敏、导出日志文件

WBP02 请先点「握手」，其余指令都依赖握手完成。PM10 与 JMRBP 不需要握手。

JMRBP 页的第 5 组「待实测项」是为真机验证准备的：协议文档没写清的几个点（记录到达顺序、
设备确认是否等价于「标记已上传」、中断后是否断点续传）各有一个按钮，点一下就能在下半屏
读到结论。把日志等级开到 `Debug` 一起看效果更好。

---

## 代码导读

按这个顺序读，能覆盖一次完整集成需要知道的全部内容：

| 文件 | 演示什么 |
| --- | --- |
| `VTMDemoCentralManager.h/.m` | **扫描与连接。这一段不属于 SDK 的职责**，需要你自己实现。集成时最容易卡住的地方。 |
| `VTMDemoDeviceViewController.m` | session 的完整生命周期：创建 → 绑定外设 → 等部署完成 → 发指令 → 页面销毁时摘回调 |
| `VTMDemoWBP02ViewController.m` | WBP02 全部对外 API，含握手链、编程写入、实时数据主动上报 |
| `VTMDemoPM10ViewController.m` | PM10 全部对外 API，含病例列表分页拉取、波形下载与进度回调 |
| `VTMDemoJMRBPViewController.m` | JMRBP 全部对外 API。这个设备没有握手、也几乎没有请求-应答配对，是「单向指令 + 主动上报」这种形态的参考；另含存储记录的幂等全量同步写法 |
| `VTMDemoLogViewController.m` | `VTMBLELogger` 的用法：等级、脱敏、实时订阅、导出文件 |
| `AppDelegate.m` | 启动时配置 SDK 日志 |

---

## 集成到你的 App

### CocoaPods

把 `VTMBLELibrary/` 目录（含 `.podspec` 与 `.xcframework`）拷到你的工程里：

```ruby
pod 'VTMBLELibrary', :path => 'VTMBLELibrary'
```

### Swift Package Manager

在 Xcode 里 File > Add Package Dependencies，填：

```
https://github.com/viatom-dev/VTMBLELibraryDemo.git
```

### 手动

把 `VTMBLELibrary.xcframework` 拖进工程，在 target 的 General >
Frameworks, Libraries, and Embedded Content 里设为 **Do Not Embed**
（这是静态 framework，不需要嵌入）。

另外需要在 Info.plist 里加 `NSBluetoothAlwaysUsageDescription`，
否则 App 一请求蓝牙权限就会被系统终止。

---

## 最小集成代码

SDK 只负责「已连接外设之上的协议层」。扫描、连接、断线重连由你的
`CBCentralManager` 管理。完整顺序：

```objc
#import <VTMBLELibrary/VTMBLELibrary.h>

// 1. 用你自己的 CBCentralManager 扫描并连接（SDK 不做这一步）

// 2. 创建 session。注意 VTMBLECoreSession 的 -init / +new 是 NS_UNAVAILABLE，
//    必须走设备类的 +session
self.session = [VTMWBP02BLESession session];

// 3. 设置 delegate
self.session.sessionDelegate = self;

// 4. 把已连接的 peripheral 交给 session。
//    ⚠️ 这个赋值会启动服务与特征值发现，是一个带副作用的 setter
self.session.peripheral = connectedPeripheral;

// 5. 等这个回调。在它到来之前发出的指令会丢，因为写入用的特征值还没找到
- (void)sessionDeployCompletion:(VTMBLECoreSession *)session {
    // 6. 现在可以发指令了
    [self.session handshakes:^(NSString *deviceSN) {
        NSLog(@"SN = %@", deviceSN);
    }];
}
```

---

## 关键约束（请务必读完）

### 必须等 `-sessionDeployCompletion:`

`session.peripheral = ...` 之后 SDK 才开始发现服务。在部署完成前调用任何
指令方法都会静默失败。

### 指令必须逐条发送

SDK 内部有一个指令队列，但**不要并发调用**：请等上一条的 callback 回来
再发下一条。示例 App 的指令列表就是按这个方式设计的。

### 线程

SDK 内部对指令队列与收包缓存没有加锁。请把「调用 SDK 的 API」和
「CoreBluetooth 的回调」固定在同一个队列上。

最简单的做法是全部放主线程：创建 `CBCentralManager` 时传
`dispatch_get_main_queue()`，示例代码就是这么做的。SDK 的 callback 会在
CoreBluetooth 的回调队列上执行，也就是你传给 central manager 的那个队列。

### callback 参数可能为 nil

头文件包在 `NS_ASSUME_NONNULL` 里，但部分 callback 实际会传 nil。
最典型的是 WBP02 的实时数据：

```objc
[session receiveRealtimeData:^(NSNumber *pressure, VTMWBP02DataModel *result) {
    // 打气过程中：(压力值, nil)
    // 测量结束时：(nil, 结果模型)
    if (pressure != nil) { /* ... */ }
    if (result   != nil) { /* ... */ }
}];
```

**Swift 调用方尤其注意**：这些参数会被导入成非可选类型，收到 nil 会直接崩溃。
这一项在下个大版本会用带 `error` 的统一签名修掉。

### 主动上报类接口只是注册，不发指令

`-receiveRealtimeData:`、`-monitorPressureEnd:` 这类方法只是把 block 存下来，
不会向设备发任何东西。请在部署完成后尽早注册，否则设备推上来的数据没人接。

### 日志与患者隐私

SDK 日志默认完全关闭。原始报文含患者生理数据，**生产环境请保持
`VTMBLELogLevelOff`**：

```objc
// 排查问题时打开
VTMBLELogger.sharedLogger.level = VTMBLELogLevelDebug;

// 需要在生产环境观察流量形态时，只保留路由字节，测量值不记录
VTMBLELogger.sharedLogger.redactsPayload = YES;

// 出问题后导出给我们
NSURL *dir = [NSFileManager.defaultManager URLsForDirectory:NSCachesDirectory
                                                 inDomains:NSUserDomainMask].firstObject;
NSError *error = nil;
NSURL *logFile = [VTMBLELogger.sharedLogger writeLogFileToDirectory:dir error:&error];
```

`handler` 回调在 logger 内部的串行队列上执行。不要在 handler 里回调
VTMBLELibrary 的任何方法。

---

## 已知限制

如实列出当前版本的不足，避免你踩坑：

- **没有超时机制。** 设备不返回预期应答时，该条指令的 callback 永远不会触发，
  且队列会卡住后续指令。请在你的业务层自己加超时。
- **没有统一错误回调。** 现有 callback 都是单参数，服务发现失败、写入失败
  这类错误不会通知调用方，只会记进 SDK 日志。
- **`peripheral` 是带副作用的 setter**，不是普通的存值属性。
- **nullability 标注与实际行为不一致**，见上面「callback 参数可能为 nil」。
- **PM10 波形解析归调用方。** `-requestCaseDataWithInfo:progressHandle:callback:`
  回调给的是原始 `NSData`，用 `VTMPM10CaseDataMdoel` 转成 µV 序列。转出来的
  `infoModel` **SDK 从不赋值，恒为 `nil`**，需要关联病例信息请自己填。
- **PM10 拉病例列表请统一用 `VTMPM10ReqCaseInfoAll`。** 设备只给「总条数」与
  「未上传条数」两个计数，SDK 拿总条数当收齐的判据，于是 `Uploaded` 可能永远凑不满
  而不回调，`Target` 也不是「只取这一条」而是从该序号继续往后拉。拿到列表后在你这边
  按 `uploadState` / `serialNumber` 过滤。
- **PM10 `supportLanguages` 后八种语言的判定待确认。** 三组语言位掩码中第二组与第三组
  读的是同一个字节，表现为 `PL`~`DE` 与 `JP`~`NL` 恒同时出现或同时缺失，暂时不要依赖。
- **JMRBP 读取存储记录不保证单次拉全。** 协议既不给总条数也不给结束标志，SDK 只能靠
  静默超时收尾，`Idle` 不等于「已拉全」。必须每次连上都无条件拉一次，按槽位 + 时间
  去重合并，不要维护同步游标 —— 一次中断过的同步若推进了游标，更早的记录会被永久跳过。
- **JMRBP 的启停指令没有 callback。** 协议未定义这几条指令的应答，所以 SDK 不提供
  回调（不给不存在的应答硬凑一个）。是否真的动起来请看压力上报。
- **JMRBP 的确认（ACK）等于告诉设备「这条你可以忘了」。** 设备收到确认后把该条记录
  标记为已上传，既不重发也不会转存为可再次读取的存储记录 —— 这条数据从此拿不回来。
  若你是在回调里才落库、而落库可能失败，请把 `acknowledgesMeasurementResult` 置为 `NO`，
  落库成功后再调 `-acknowledgeMeasurementResult`。Demo 的 JMRBP 页第 4 组可以直接切换。
- **JMRBP 当前读不到存储记录。** 样机对读取指令零响应（厂商 demo 同样读不到），
  怀疑该指令有协议文档未描述的前置条件，已在向厂商确认。
- **JMRBP 有几个字节的含义待实测确认**，压力换算系数与四个 `reserved` 字节属于此类，
  当前按原始码值透出。
- **结论文案只内置 ZH / EN 两种**，而 PM10 设备本身支持 18 种语言。需要覆盖更多
  语言时，请拿 `resultCodes` 的原始码值自己做本地化。
- 部分已发布 API 存在拼写错误（`getBatteyInfo:`、`daylightEntTime`、
  `VTMPM10CaseDataMdoel`、`initWitData:`）。为了不破坏现有集成方的编译，
  这些名字不会修改。

---

## 维护者

以下内容需要访问私有源码仓，第三方集成方可以跳过。

### 只改了 Demo 代码

正常提交推送即可，二进制不用动。

### 改了 SDK 源码

```bash
# 1. 在私有仓改代码，用源码模式在 Demo 里断点调试
cd VTMBLELibraryDemo
VTM_BLE_SDK_SOURCE=../path/to/vtmblelibrary pod install

# 2. 改完，在私有仓提交并推到内网

# 3. 同步到本仓库：重新打包 → 拷贝 → 切回二进制模式 → 编译验证
cd ../path/to/vtmblelibrary
./Scripts/sync-demo.sh

# 4. 按脚本最后的提示提交
```

**发版前必须在二进制模式下编译一次。** 公开头文件漏登记、公开头文件 import
了私有头文件这两类问题，在源码模式下永远不会暴露。`sync-demo.sh` 会强制走这一步。

### 发版

版本号抄在四个地方，手工维护必然漂移。发版前先跑一致性检查：

```bash
cd /path/to/vtmblelibrary
./Scripts/check-versions.sh
```

它校验私有仓的 `MARKETING_VERSION`（Debug / Release 两处必须相同）、
源码 podspec、本仓库的二进制 podspec、以及 CHANGELOG 最新发布版本号是否一致。
`sync-demo.sh` 会先跑它，不一致直接中止。

注意 `MARKETING_VERSION` 会嵌进 framework 的 `Info.plist`，改了版本号必须重新打包，
否则集成方查到的版本和你以为的不一致。

版本号定稿后，两个仓打同名 tag（统一带 `v` 前缀）：

```bash
git tag -a v1.1.0 -m "..."   # 私有仓与本仓库都要打
git push origin v1.1.0
```

### 本地提交守卫

Demo 仓装了一个 `pre-commit` 钩子，拦两类「本地能跑、公开出去就坏」的错误：
源码模式的 `Podfile.lock`（含仓库外路径，第三方 `pod install` 会失败）、
私有头文件混进 XCFramework。

钩子在 `.git/hooks/` 下，不随仓库分发。换机器或重新 clone 后重新装：

```bash
./Scripts/install-demo-hooks.sh /path/to/VTMBLELibraryDemo
```

---

## 版本

当前 `1.1.0`，对应 tag `v1.1.0`。支持 `ios-arm64` 与 `ios-arm64_x86_64-simulator`。

版本号也嵌在二进制里，反馈问题时可以这样确认你手上是哪一版：

```bash
/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' \
  VTMBLELibrary/VTMBLELibrary.xcframework/ios-arm64/VTMBLELibrary.framework/Info.plist
```

版本号遵循[语义化版本](https://semver.org/lang/zh-CN/)。公开 API 的破坏性变更会升 major，
届时本文件的「已知限制」一节会同步更新。

## 许可

示例 App 源码以 MIT 许可发布。`VTMBLELibrary.xcframework` 为
Viatom 专有软件，条款见 [LICENSE](LICENSE)。

## 联系

技术问题请提 Issue，或发邮件到 ios@viatomtech.com。
提问时请附上导出的 SDK 日志文件、设备型号与 SDK 版本号。
