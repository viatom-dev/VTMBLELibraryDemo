//
//  VTMPM10BLESession.h
//  VTMBLELibrary
//
//  Created by yangweichao on 2025/11/7.
//
//  注意：指令必须逐条发送，不要同时发多条。
//  PS: commands go one by one. Do not send multiple commands at the same time.
//

#import <VTMBLELibrary/VTMBLECoreSession.h>
#import <VTMBLELibrary/VTMPM10Enum.h>
@class VTMPM10DeviceModel,VTMPM10CaseInfoModel,VTMPM10SettingsModel;

NS_ASSUME_NONNULL_BEGIN

/// @brief PM10 手持心电的会话。
///
/// Session for the PM10 handheld ECG recorder.
///
/// @discussion 用法：自行用 `CBCentralManager` 连上设备，把 `CBPeripheral` 赋给
/// `peripheral`，等 `-sessionDeployCompletion:` 回来之后即可发指令 ——
/// 该设备**没有握手流程**。
///
/// Usage: connect the device with your own `CBCentralManager`, assign the
/// `CBPeripheral` to `peripheral`, and start issuing commands once
/// `-sessionDeployCompletion:` arrives. This device has **no handshake step**.
///
/// 典型顺序：校时 → 读参数 → 拉病例列表 → 逐条下载波形。
///
/// A typical sequence: sync the clock, read the settings, fetch the case list,
/// then download waveforms one case at a time.
///
/// @warning 指令必须逐条发送，上一条的回调到达之前不要发下一条。
/// 所有 block 都在 `CBCentralManager` 所在队列上回调，更新 UI 前请自行切回主队列。
///
/// Commands must be issued one at a time; do not start the next one before the
/// previous callback arrives. Every block is invoked on the queue you handed to
/// `CBCentralManager`, so hop back to the main queue before touching UI.
@interface VTMPM10BLESession : VTMBLECoreSession

/// @brief 创建一个会话。每次调用都返回新实例。
///
/// Creates a session. Each call returns a new instance.
///
/// @discussion 名字读起来像单例，但它不是。保留这个名字是因为 Swift 的 ObjC 导入规则
/// 会把它映射成 `VTMPM10BLESession()`。
///
/// The name reads like a singleton accessor but it is not one. It is kept
/// because Swift's ObjC import rules map it to `VTMPM10BLESession()`.
+ (instancetype)session;

#pragma mark - 设备信息 / Device info

/// @brief 读取设备标识与版本信息。
///
/// Reads the device identity and firmware versions.
///
/// @discussion 内部连发三条指令（产品标识 → 版本 → 序列号），三条应答都收到之后
/// 回调一次聚合好的模型。
///
/// Internally issues three commands in a row (product logo, versions, serial
/// number) and fires the callback once, with a single aggregated model, after
/// all three responses have arrived.
///
/// @param callback 聚合后的设备信息。/ The aggregated device info.
- (void)requestDeviceInfo:(void(^)(VTMPM10DeviceModel *deviceModel))callback;

#pragma mark - 校时 / Clock

/// @brief 给设备校时。
///
/// Sets the device clock.
///
/// @discussion 病例的时间戳由设备时钟给出，建议在读取病例列表之前先校一次时。
///
/// Case timestamps come from the device clock, so sync it before reading the
/// case list.
///
/// @param date 要写入设备的时间，精确到毫秒。传 `nil` 使用当前系统时间。
///             The time to write, down to milliseconds. Pass `nil` to use the
///             current system time.
/// @param callback `@1` 成功，`@0` 失败（设备返回了非 0 的结果码）。
///                 `@1` on success, `@0` on failure (the device answered with a
///                 non-zero result code).
- (void)syncDate:(NSDate * _Nullable)date callback:(void(^)(NSNumber *result))callback;

#pragma mark - 病例 / Cases

/// @brief 拉取病例列表。
///
/// Fetches the case list.
///
/// @discussion 内部先问设备总条数，再逐条拉取，收齐后一次性回调。设备上没有病例时
/// 立即回调空数组。
///
/// Internally asks the device for a total count first, then walks the list entry
/// by entry and fires the callback once everything has been collected. When the
/// device holds no cases the callback fires immediately with an empty array.
///
/// @warning 一次调用可能与设备往返上百次。列表很长时这个过程不快，且期间不要发别的指令。
/// 另外 `Uploaded` 与 `Target` 两种方式的行为有已知偏差，见 `VTMPM10ReqCaseInfo`
/// 上的说明 —— 实践中建议用 `All` 后在调用方过滤。
///
/// One call can exchange hundreds of frames with the device. It is not fast for
/// long lists, and no other command may be issued while it runs. Note also that
/// the `Uploaded` and `Target` modes behave differently from what their names
/// suggest; see the notes on `VTMPM10ReqCaseInfo`. In practice, prefer `All` and
/// filter on your side.
///
/// @param method 拉取范围。/ Which cases to enumerate.
/// @param sn 仅 `VTMPM10ReqCaseInfoTarget` 用得上，其余方式传 `0`。
///           Only used with `VTMPM10ReqCaseInfoTarget`; pass `0` otherwise.
/// @param callback 元素为 `VTMPM10CaseInfoModel` 的数组。
///                 An array of `VTMPM10CaseInfoModel`.
- (void)requestCaseInfo:(VTMPM10ReqCaseInfo)method serialNumber:(NSUInteger)sn callback:(void(^)(NSArray <VTMPM10CaseInfoModel *>*infos))callback;

/// @brief 下载一条病例的心电波形数据。
///
/// Downloads the ECG waveform of one case.
///
/// @discussion 回调给出的是**未解析的原始数据**，用
/// `VTMPM10CaseDataMdoel` 转成 µV 序列：
///
/// The callback hands back **raw, undecoded data**. Turn it into a µV sequence
/// with `VTMPM10CaseDataMdoel`:
///
/// ```
/// [session requestCaseDataWithInfo:info progressHandle:^(CGFloat progress) {
///     // 0.0 ~ 1.0
/// } callback:^(NSData *data) {
///     VTMPM10CaseDataMdoel *ecg = [[VTMPM10CaseDataMdoel alloc] initWitData:data];
///     NSArray<NSNumber *> *microVolts = ecg.uVElements;
/// }];
/// ```
///
/// @warning 下载依赖 `model` 的 `serialNumber` 与 `length`，请直接使用
/// `-requestCaseInfo:serialNumber:callback:` 给出的模型，不要自己拼一个。
/// 设备中途停止推送时不会有任何通知，回调永远不来 —— 库暂无超时机制，
/// 业务侧需要自行加超时兜底。
///
/// The download relies on `serialNumber` and `length` from `model`, so pass the
/// model you got from `-requestCaseInfo:serialNumber:callback:` rather than
/// building one yourself. If the device stops pushing mid-transfer there is no
/// notification and the callback never fires — the library has no timeout yet,
/// so add one on your side.
///
/// @param model 要下载的病例。/ The case to download.
/// @param handle 下载进度，`0.0` ~ `1.0`。回调频率约为每 6 个采样点一次。
///               Download progress from `0.0` to `1.0`, reported roughly once
///               per 6 samples.
/// @param callback 下载完成后的完整原始数据。/ The complete raw data once the transfer finishes.
- (void)requestCaseDataWithInfo:(VTMPM10CaseInfoModel *)model progressHandle:(void(^)(CGFloat progress))handle callback:(void(^)(NSData *data))callback;

#pragma mark - 设备参数 / Settings

/// @brief 读取设备的测量参数设置。
///
/// Reads the device's measurement settings.
///
/// @discussion 要改设置时先调用它拿到一份完整模型，再改需要改的字段。
///
/// Call it first to obtain a complete model, then change only the fields you
/// care about.
///
/// @param callback 当前设置，含该固件支持的语言列表。
///                 The current settings, including the language list this
///                 firmware supports.
- (void)requestParameters:(void(^)(VTMPM10SettingsModel *model))callback;

/// @brief 写入设备的测量参数设置。
///
/// Writes the device's measurement settings.
///
/// @warning 写入是**整份覆盖**。请传 `-requestParameters:` 读回并修改过的模型，
/// 不要 new 一个空模型 —— 未设置的字段会被清零。
///
/// Writing **replaces every field**. Pass the model you read back with
/// `-requestParameters:` and modified; do not build a fresh empty one, or the
/// fields you did not set will be zeroed.
///
/// @param model 完整的设置模型。/ A complete settings model.
/// @param callback `@1` 成功，`@0` 失败（设备返回了非 0 的结果码）。
///                 `@1` on success, `@0` on failure (the device answered with a
///                 non-zero result code).
- (void)syncParameters:(VTMPM10SettingsModel *)model callback:(void(^)(NSNumber *result))callback;

@end

NS_ASSUME_NONNULL_END
