//
//  VTMWBP02BLESession.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/13.
//

#import <VTMBLELibrary/VTMBLECoreSession.h>

@class VTMWBP02DataModel, VTMWBP02AutoModeState, VTMWBP02BatteryModel;

NS_ASSUME_NONNULL_BEGIN

/// @brief WBP02 动态血压计的会话。
///
/// Session for the WBP02 ambulatory blood pressure monitor.
///
/// @discussion 用法：自行用 `CBCentralManager` 连上设备，把 `CBPeripheral` 赋给
/// `peripheral`，等 `-sessionDeployCompletion:` 回来之后先调用 `-handshakes:`，
/// 再调用其余方法。
///
/// Usage: connect the device with your own `CBCentralManager`, assign the
/// `CBPeripheral` to `peripheral`, wait for `-sessionDeployCompletion:`, then
/// call `-handshakes:` first and everything else after it.
///
/// @warning 指令必须逐条发送，上一条的回调到达之前不要发下一条。
/// 所有 block 都在 `CBCentralManager` 所在队列上回调，更新 UI 前请自行切回主队列。
///
/// Commands must be issued one at a time; do not start the next one before the
/// previous callback arrives. Every block is invoked on the queue you handed to
/// `CBCentralManager`, so hop back to the main queue before touching UI.
@interface VTMWBP02BLESession : VTMBLECoreSession

/// @brief 创建一个会话。每次调用都返回新实例。
///
/// Creates a session. Each call returns a new instance.
///
/// @discussion 名字读起来像单例，但它不是。保留这个名字是因为 Swift 的 ObjC 导入规则
/// 会把它映射成 `VTMWBP02BLESession()`，改名反而会破坏 Swift 侧的惯用写法。
///
/// The name reads like a singleton accessor but it is not one. It is kept
/// because Swift's ObjC import rules map it to `VTMWBP02BLESession()`; renaming
/// it would break the idiomatic Swift call site.
+ (instancetype)session;

#pragma mark - 握手 / Handshake

/// @brief 与设备握手，回调给出设备序列号。
///
/// Performs the handshake and reports the device serial number.
///
/// @discussion 部署完成后要调用的第一个方法。库内部依次完成
/// 握手 → 校时 → 设置登录用户 → 读设备信息四步，中间步骤不占用指令队列，
/// 只有最后一步的应答才触发回调。校时用的是手机当前时间。
///
/// This is the first method to call after deployment. Internally it runs four
/// steps in order — handshake, clock sync, set login user, read device info —
/// where the intermediate steps do not occupy the command queue; only the
/// response to the last step fires the callback. The clock is set from the
/// phone's current time.
///
/// @param callback 设备序列号，由应答报文按 UTF-8 解码得到。
///                 The device serial number, decoded from the response as UTF-8.
- (void)handshakes:(void(^)(NSString *deviceSN))callback;

#pragma mark - 历史记录 / Stored history

/// @brief 读取设备存储的血压记录。
///
/// Reads the blood pressure records stored on the device.
///
/// @discussion 一次读取会与设备往返多次，库内部逐条收齐后一次性回调。
///
/// A single read exchanges many frames with the device; the library collects
/// every record and fires the callback once at the end.
///
/// @warning 库会丢弃 `errorCode != 0` 的无效记录，因此回调给出的条数可能少于
/// 设备上实际存储的条数，且每条记录的 `errorCode` 恒为 `0`。
///
/// Invalid records (`errorCode != 0`) are dropped, so the callback can report
/// fewer records than the device actually holds, and every returned record has
/// `errorCode` equal to `0`.
///
/// @param date 起始时间，只用到年月日时分。传 `nil` 读取全部记录。
///             Lower time bound; only year, month, day, hour and minute are
///             used. Pass `nil` to read every record.
/// @param callback 元素为 `VTMWBP02DataModel` 的数组，没有记录时为空数组。
///                 An array of `VTMWBP02DataModel`, empty when there is nothing
///                 to report.
- (void)getHistory:(NSDate *)date callback:(void(^)(NSArray *results))callback;

#pragma mark - 电量 / Battery

/// @brief 读取电池状态。
///
/// Reads the battery state.
///
/// @discussion 方法名少了一个字母（`Battey` 应为 `Battery`）。这是已发布的公开 API，
/// 为不破坏集成方编译而保留原拼写。
///
/// The selector is misspelled (`Battey` should be `Battery`). It is already
/// published API and is kept as-is so that integrator code keeps compiling.
///
/// @warning 该 callback 会被库长期持有：设备在测量过程中主动上报电量时，
/// **同一个 block 会再次被调用**，而不只是本次请求的应答。若 block 里捕获了
/// 视图控制器，请注意循环引用与生命周期。
///
/// The callback is retained by the library: when the device pushes its battery
/// state on its own during a measurement, **the same block is invoked again**,
/// not just for the response to this request. Mind retain cycles and object
/// lifetime if the block captures a view controller.
///
/// @param callback 电池状态。/ The battery state.
- (void)getBatteyInfo:(void(^)(VTMWBP02BatteryModel *model))callback;

#pragma mark - 自动测量设置 / Automatic measurement settings

/// @brief 开关白天时段的自动测量。
///
/// Turns daytime automatic measurement on or off.
///
/// @param autoMode `YES` 开启，`NO` 关闭。/ `YES` to enable, `NO` to disable.
/// @param callback 设备接受设置后回调，参数**恒为 `@YES`**，表示「已下发成功」，
///                 不是回读设备上的实际值。想确认实际值请调用
///                 `-getAutoModeSetiingInfo:`。
///                 Invoked once the device accepts the setting. The argument is
///                 **always `@YES`**, meaning "the write went through" — it is
///                 not a read-back of the device state. Use
///                 `-getAutoModeSetiingInfo:` to confirm the actual value.
- (void)setDaylightAutoMode:(BOOL)autoMode callback:(void(^)(NSNumber *autoMode))callback;

/// @brief 设置白天时段的起止时间与测量间隔。
///
/// Sets the daytime window and its measurement interval.
///
/// @param startTime 开始时间，格式 `HH:mm`。/ Start time, formatted as `HH:mm`.
/// @param endTime 结束时间，格式 `HH:mm`。/ End time, formatted as `HH:mm`.
/// @param timeInterval 测量间隔，单位分钟。/ Measurement interval, in minutes.
/// @param callback 设备接受设置后回调，参数恒为 `@YES`。
///                 Invoked once the device accepts the setting; always `@YES`.
- (void)setDaylightAutoStartTime:(NSString *)startTime endTime:(NSString *)endTime timeInterval:(NSInteger)timeInterval callback:(void(^)(NSNumber *success))callback;

/// @brief 开关夜间时段的自动测量。
///
/// Turns night-time automatic measurement on or off.
///
/// @param autoMode `YES` 开启，`NO` 关闭。/ `YES` to enable, `NO` to disable.
/// @param callback 设备接受设置后回调，参数恒为 `@YES`。
///                 Invoked once the device accepts the setting; always `@YES`.
- (void)setNightlyAutoMode:(BOOL)autoMode callback:(void(^)(NSNumber *autoMode))callback;

/// @brief 设置夜间时段的起止时间与测量间隔。
///
/// Sets the night-time window and its measurement interval.
///
/// @param startTime 开始时间，格式 `HH:mm`。/ Start time, formatted as `HH:mm`.
/// @param endTime 结束时间，格式 `HH:mm`。/ End time, formatted as `HH:mm`.
/// @param timeInterval 测量间隔，单位分钟。/ Measurement interval, in minutes.
/// @param callback 设备接受设置后回调，参数恒为 `@YES`。
///                 Invoked once the device accepts the setting; always `@YES`.
- (void)setNightlyAutoStartTime:(NSString *)startTime endTime:(NSString *)endTime timeInterval:(NSInteger)timeInterval callback:(void(^)(NSNumber *success))callback ;

/// @brief 读回设备当前的自动测量设置。
///
/// Reads back the device's current automatic measurement settings.
///
/// @discussion 方法名少了一个字母（`Setiing` 应为 `Setting`）。这是已发布的公开 API，
/// 为不破坏集成方编译而保留原拼写。
///
/// The selector is misspelled (`Setiing` should be `Setting`). It is already
/// published API and is kept as-is so that integrator code keeps compiling.
///
/// @param callback 白天与夜间两套配置。/ Both the daytime and the night-time configuration.
- (void)getAutoModeSetiingInfo:(void(^)(VTMWBP02AutoModeState *autoState))callback;

#pragma mark - 主动上报 / Device-initiated reports

/// @brief 注册测量过程的上报。设备加压时持续推送袖带压，测量结束时推送结果。
///
/// Registers for measurement reports: the cuff pressure while the device
/// inflates, and the result when the measurement ends.
///
/// @discussion 只做注册，不发送任何指令。建议在 `-handshakes:` 的回调里尽早注册，
/// 否则设备自行发起的测量可能观察不到。
///
/// This only registers the block; no command is sent. Register it as early as
/// possible, ideally inside the `-handshakes:` callback, otherwise a
/// measurement started on the device itself may go unobserved.
///
/// @warning **两个参数不会同时有值。** 加压过程中只有 `pressure`，`result` 为 `nil`；
/// 测量结束时只有 `result`，`pressure` 为 `nil`。两者都声明在
/// `NS_ASSUME_NONNULL` 之内但实际会传 `nil`，因此 Objective-C 侧使用前必须判空，
/// **Swift 侧会直接 trap** —— 这是历史遗留的签名缺陷，会在下一个大版本拆成两个方法。
///
/// **The two arguments are never both present.** While the cuff inflates only
/// `pressure` is set and `result` is `nil`; when the measurement ends only
/// `result` is set and `pressure` is `nil`. Both are declared inside
/// `NS_ASSUME_NONNULL` yet `nil` is actually passed, so Objective-C callers
/// must nil-check and **Swift callers will trap**. This is a legacy signature
/// flaw, to be split into two methods in the next major version.
///
/// @param realtimeData `pressure` 为实时袖带压，`result` 为本次测量结果。
///                     `pressure` is the live cuff pressure, `result` is the
///                     finished measurement.
- (void)receiveRealtimeData:(void(^)(NSNumber *pressure, VTMWBP02DataModel *result))realtimeData;

/// @brief 注册「打气被手动停止」的上报。
///
/// Registers for the "inflation stopped by hand" report.
///
/// @discussion 只做注册，不发送任何指令。用户在设备上手动中止加压时设备会上报一次。
///
/// This only registers the block; no command is sent. The device reports once
/// when the user aborts the inflation on the device itself.
///
/// @param callback 参数恒为 `YES`，只表示「这件事发生了」。
///                 Always `YES`; the argument only signals that the event
///                 happened.
- (void)monitorPressureEnd:(void(^)(BOOL end))callback;

@end

NS_ASSUME_NONNULL_END
