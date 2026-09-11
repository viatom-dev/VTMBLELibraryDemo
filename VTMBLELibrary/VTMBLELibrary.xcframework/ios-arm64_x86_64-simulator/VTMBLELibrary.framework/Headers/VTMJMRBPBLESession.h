//
//  VTMJMRBPBLESession.h
//  VTMBLELibrary
//
//  Created by viatom on 2026/9/8.
//

#import <VTMBLELibrary/VTMBLECoreSession.h>
#import <VTMBLELibrary/VTMJMRBPEnum.h>

@class VTMJMRBPRecordModel, VTMJMRBPPressureModel;

NS_ASSUME_NONNULL_BEGIN

/// @brief 蓝牙血压计会话。
///
/// Session for the JMRBP Bluetooth blood pressure monitor.
///
/// @discussion 用法：自行用 `CBCentralManager` 连上设备，把 `CBPeripheral` 赋给
/// `peripheral`，等 `-sessionDeployCompletion:` 回来之后再调用本类的方法。
///
/// Usage: connect the device with your own `CBCentralManager`, assign the
/// `CBPeripheral` to `peripheral`, and wait for `-sessionDeployCompletion:`
/// before calling anything on this class.
///
/// 该设备的协议几乎全是单向报文：除了读取存储记录，其余操作设备都不给应答，
/// 因此那些方法没有 callback —— 发出即返回。测量过程通过 `-receiveRealtimePressure:`
/// 等注册的上报 block 观察。
///
/// This device's protocol is almost entirely one-way: apart from reading stored
/// records, the device answers nothing, so those methods take no callback — they
/// return as soon as the frame goes out. Observe a measurement through the
/// report blocks you register, such as `-receiveRealtimePressure:`.
///
/// @warning 除读取存储记录的 completion 之外，所有 block 都在 `CBCentralManager`
/// 所在队列上回调。
///
/// Except for the history-read completion, every block is invoked on the queue
/// you handed to `CBCentralManager`.
@interface VTMJMRBPBLESession : VTMBLECoreSession

/// @brief 创建一个会话。每次调用都返回新实例。
///
/// Creates a session. Each call returns a new instance.
+ (instancetype)session;

#pragma mark - 设备操作 / Device control

/// @brief 让设备进入工作状态，使用设备自身的默认配置。
///
/// Puts the device into working state using its own default profile.
///
/// @warning 该操作会驱动设备硬件对使用者的手臂执行一次加压流程。
/// 请只在用户明确操作时调用，不要用于探活或状态轮询。
///
/// This drives the device hardware through a real inflation cycle on the user's
/// arm. Call it only in response to an explicit user action; never use it as a
/// liveness check or for state polling.
- (void)activate;

/// @brief 让设备以指定配置进入工作状态。
///
/// Puts the device into working state with the given profile.
///
/// @warning 同 `-activate`，会驱动设备硬件对使用者加压。
///
/// Like `-activate`, this inflates the cuff on the user's arm.
///
/// @param profile 工作配置。设备不支持的配置通常会被忽略并按默认配置执行。
///                The working profile. A profile the device does not support is
///                usually ignored and the default is used instead.
- (void)activateWithProfile:(VTMJMRBPProfile)profile;

/// @brief 中止设备当前的工作状态。
///
/// Aborts whatever the device is currently doing.
///
/// @discussion 设备不返回确认，是否真的停下来要通过压力上报是否终止来判断。
///
/// The device sends no confirmation; tell whether it actually stopped by
/// watching for the pressure reports to cease.
- (void)deactivate;

#pragma mark - 设备配置 / Device configuration

/// @brief 下发设备时钟。
///
/// Writes the device clock.
///
/// @discussion 建议在 `-sessionDeployCompletion:` 之后、读取存储记录之前调用一次。
/// 记录里的时间戳由设备时钟给出，设备断电后可能归零，不校时会导致记录时间不可用。
///
/// Call it once after `-sessionDeployCompletion:` and before reading stored
/// records. Record timestamps come from the device clock, which can reset to
/// zero after a power loss; without a sync the timestamps are unusable.
///
/// @param date 要写入设备的时间。传 `nil` 使用当前系统时间。
///             The time to write. Pass `nil` to use the current system time.
- (void)syncTime:(nullable NSDate *)date;

#pragma mark - 存储记录 / Stored records

/// @brief 读取指定槽位的存储记录。
///
/// Reads the records stored in the given user slot.
///
/// @discussion 设备**既不提供总条数，也不提供传输结束标志**，因此本方法无法保证一次拉全。
/// 正确的用法是幂等全量：每次连接后无条件拉一次，按 `user` + `date` 去重合并进本地库，
/// 不要维护「上次同步到哪」的游标。设备上的记录在显式删除前一直保留，
/// 这一轮没取到的下一轮会重新出现。
///
/// The device provides **neither a total count nor an end-of-transfer marker**,
/// so this method cannot guarantee a complete read. The correct usage is an
/// idempotent full sync: fetch unconditionally after every connection and merge
/// into your local store, deduplicating on `user` + `date`. Do not keep a
/// "synced up to here" cursor. Records stay on the device until they are
/// explicitly deleted, so whatever this round missed will reappear in the next.
///
/// 库内部会过滤掉同一轮中设备重发的重复记录，但**不做跨轮次去重** —— 库不做持久化，
/// 跨轮次去重是调用方数据库的职责。
///
/// The library filters out duplicates the device resends **within one round**
/// but does **not** deduplicate across rounds: it stores nothing persistently,
/// so cross-round deduplication belongs to the caller's database.
///
/// @param user 要读取的槽位。/ The slot to read.
/// @param onRecord 每收到一条即回调。**请在此即时落库**，这样中途断开时已收到的记录不会丢。
///                 Invoked for every record as it arrives. **Persist it right
///                 here**, so that nothing already received is lost if the
///                 connection drops mid-round.
/// @param completion 本轮结束时回调。`records` 是本轮去重后的全部记录，
///                   `reason` 说明结束原因，据此判断是否需要重试。
///                   **该 block 在主队列回调**，与其他 block 不同。
///                   Invoked when the round ends. `records` holds every
///                   deduplicated record of this round and `reason` explains why
///                   it ended, which is what you decide retries on. **This block
///                   is invoked on the main queue**, unlike the others.
- (void)getHistoryForUser:(VTMJMRBPUser)user
                   record:(nullable void(^)(VTMJMRBPRecordModel *record))onRecord
               completion:(nullable void(^)(NSArray<VTMJMRBPRecordModel *> *records,
                                            VTMJMRBPHistoryEndReason reason))completion;

/// @brief 中断正在进行的存储记录读取。
///
/// Interrupts a stored-record read in progress.
///
/// @discussion 实现方式是停止向设备确认，设备约 3 秒后自行终止上传。
/// 调用后 completion 会立即以 `VTMJMRBPHistoryEndReasonCancelled` 回调。
/// 没有进行中的读取时本方法无副作用。
///
/// It works by stopping the acknowledgements, after which the device gives up on
/// its own in about 3 seconds. The completion fires immediately with
/// `VTMJMRBPHistoryEndReasonCancelled`. Calling it while no read is running has
/// no effect.
- (void)cancelHistorySync;

/// @brief 删除设备端全部槽位的存储记录。
///
/// Deletes the stored records of every slot on the device.
///
/// @warning **不可逆，且设备不返回确认。** 由于无法确认上一次读取是否完整，
/// 删除前请确保调用方已经拿到并持久化了需要的记录。测量进行中设备会忽略该指令。
///
/// **Irreversible, and the device sends no confirmation.** Because a previous
/// read cannot be confirmed complete, make sure you have received and persisted
/// everything you need before calling it. The device ignores the command while a
/// measurement is running.
- (void)deleteAllHistory;

#pragma mark - 主动上报 / Device-initiated reports

/// @brief 注册实时压力上报。测量开始后设备持续推送袖带压力。
///
/// Registers for live pressure reports. The device pushes the cuff pressure
/// continuously once a measurement starts.
///
/// @discussion 只做注册，不发送任何指令。
///
/// This only registers the block; no command is sent.
- (void)receiveRealtimePressure:(void(^)(VTMJMRBPPressureModel *pressure))block;

/// @brief 注册实时测量结果上报。一次测量正常结束时回调。
///
/// Registers for live measurement results, invoked when a measurement finishes
/// normally.
///
/// @discussion 只做注册，不发送任何指令。
/// 设备存储的历史记录不走这里，走 `-getHistoryForUser:record:completion:`。
///
/// This only registers the block; no command is sent. Records stored on the
/// device do not come through here — use
/// `-getHistoryForUser:record:completion:` for those.
- (void)receiveMeasurementResult:(void(^)(VTMJMRBPRecordModel *record))block;

/// @brief 注册故障上报。测量失败时设备主动上报故障码。
///
/// Registers for fault reports. The device pushes a fault code when a
/// measurement fails.
///
/// @discussion 只做注册，不发送任何指令。回调给出的是**设备上报的原始码值**，
/// 库不附带文案。码值含义见随附的协议对照表，文案与本地化由调用方处理。
///
/// This only registers the block; no command is sent. The callback carries the
/// **raw code the device reported**; the library ships no wording for it. See
/// the accompanying protocol table for the meanings, and handle wording and
/// localization on your side.
- (void)receiveFaultCode:(void(^)(NSUInteger code))block;

#pragma mark - 实时结果的确认 / Acknowledging live results

/// @brief 是否由 SDK 自动确认实时测量结果。默认 `YES`。
///
/// Whether the SDK acknowledges live measurement results automatically.
/// Defaults to `YES`.
///
/// @discussion **确认在设备侧等于「这条你可以忘了」。** 设备收到确认后会把该条记录
/// 标记为已上传：不再重发，也**不会**转存为可通过
/// `-getHistoryForUser:record:completion:` 再次读取的存储记录。
///
/// **On the device side, an acknowledgement means "you may forget this one".**
/// Once acknowledged, the device marks the record as uploaded: it stops
/// resending it and does **not** convert it into a stored record that
/// `-getHistoryForUser:record:completion:` could read back.
///
/// 因此当调用方是在收到回调之后才落库、而落库有失败的可能时（写库失败、进程被杀、
/// 上传服务端失败），自动确认相当于 SDK 替调用方做出了一个它无法保证的承诺，
/// 一旦落库没成功，这条测量数据就再也拿不回来了。这种场景请置为 `NO`，
/// 在数据确实落库之后调用 `-acknowledgeMeasurementResult`。
///
/// So if you persist the record only after the callback arrives and persisting
/// can fail (a failed write, a killed process, a failed upload), automatic
/// acknowledgement has the SDK make a promise on your behalf that it cannot
/// keep: should the write fail, that measurement is gone for good. Set this to
/// `NO` in that case and call `-acknowledgeMeasurementResult` once the data is
/// genuinely stored.
///
/// 置为 `NO` 且始终不确认时，设备会每隔约 1 秒重发同一条结果、共 3 次
/// —— 也就是 `-receiveMeasurementResult:` 注册的 block 会被调用 3 次，
/// **调用方需要自行按 `user` + `date` 去重** —— 之后设备把它转存为存储记录。
///
/// With this set to `NO` and no acknowledgement ever sent, the device resends
/// the same result about once a second, three times over — meaning the block
/// registered through `-receiveMeasurementResult:` is invoked three times and
/// **you have to deduplicate on `user` + `date` yourself** — after which the
/// device converts it into a stored record.
///
/// @warning 该开关**只作用于实时测量结果**。读取存储记录时的确认是无条件发送的，
/// 因为设备靠它推进到下一条，不确认会让这一轮读取停在原地。
///
/// This switch affects **live measurement results only**. Acknowledgements sent
/// while reading stored records are unconditional, because the device relies on
/// them to advance to the next record; withholding one stalls the round.
@property (nonatomic, assign) BOOL acknowledgesMeasurementResult;

/// @brief 确认当前待确认的实时测量结果。
///
/// Acknowledges the live measurement result that is currently pending.
///
/// @discussion 供 `acknowledgesMeasurementResult` 为 `NO` 时使用：在数据确实落库之后
/// 调用，设备收到后才会把该条标记为已上传。
///
/// For use when `acknowledgesMeasurementResult` is `NO`: call it once the data
/// is genuinely stored, and only then does the device mark the record as
/// uploaded.
///
/// 设备的重发窗口约 3 秒，超时后它会自行把记录转存为存储记录；此时再调用本方法
/// 不会有效果，也不会有副作用。没有待确认的结果时调用本方法同样无副作用。
///
/// The device's resend window is about 3 seconds; after that it converts the
/// record into a stored record on its own, and calling this method has neither
/// effect nor side effects. The same is true when nothing is pending.
///
/// @warning 读取存储记录期间本方法会被拒绝并记一条警告。确认报文不带任何标识，
/// 设备只会把它算在「当前正在等待确认的那条」上，此时插入确认会让设备提前推进队列、
/// 跳过一条记录。
///
/// While stored records are being read this method is rejected and a warning is
/// logged. The acknowledgement frame carries no identifier, so the device
/// applies it to whatever record it is currently waiting on; slipping one in
/// here would make it advance early and skip a record.
- (void)acknowledgeMeasurementResult;

@end

NS_ASSUME_NONNULL_END
