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
/// @discussion 用法：自行用 `CBCentralManager` 连上设备，把 `CBPeripheral` 赋给
/// `peripheral`，等 `-sessionDeployCompletion:` 回来之后再调用本类的方法。
///
/// 该设备的协议几乎全是单向报文：除了读取存储记录，其余操作设备都不给应答，
/// 因此那些方法没有 callback —— 发出即返回。测量过程通过 `-receiveRealtimePressure:`
/// 等注册的上报 block 观察。
///
/// @warning 除读取存储记录的 completion 之外，所有 block 都在 `CBCentralManager`
/// 所在队列上回调。
@interface VTMJMRBPBLESession : VTMBLECoreSession

+ (instancetype)session;

#pragma mark - 设备操作

/// @brief 让设备进入工作状态，使用设备自身的默认配置。
///
/// @warning 该操作会驱动设备硬件对使用者的手臂执行一次加压流程。
/// 请只在用户明确操作时调用，不要用于探活或状态轮询。
- (void)activate;

/// @brief 让设备以指定配置进入工作状态。
///
/// @warning 同 `-activate`，会驱动设备硬件对使用者加压。
/// @param profile 工作配置。设备不支持的配置通常会被忽略并按默认配置执行。
- (void)activateWithProfile:(VTMJMRBPProfile)profile;

/// @brief 中止设备当前的工作状态。
///
/// @discussion 设备不返回确认，是否真的停下来要通过压力上报是否终止来判断。
- (void)deactivate;

#pragma mark - 设备配置

/// @brief 下发设备时钟。
///
/// @discussion 建议在 `-sessionDeployCompletion:` 之后、读取存储记录之前调用一次。
/// 记录里的时间戳由设备时钟给出，设备断电后可能归零，不校时会导致记录时间不可用。
/// @param date 要写入设备的时间。传 `nil` 使用当前系统时间。
- (void)syncTime:(nullable NSDate *)date;

#pragma mark - 存储记录

/// @brief 读取指定槽位的存储记录。
///
/// @discussion 设备**既不提供总条数，也不提供传输结束标志**，因此本方法无法保证一次拉全。
/// 正确的用法是幂等全量：每次连接后无条件拉一次，按 `user` + `date` 去重合并进本地库，
/// 不要维护「上次同步到哪」的游标。设备上的记录在显式删除前一直保留，
/// 这一轮没取到的下一轮会重新出现。
///
/// 库内部会过滤掉同一轮中设备重发的重复记录，但**不做跨轮次去重** —— 库不做持久化，
/// 跨轮次去重是调用方数据库的职责。
///
/// @param user 要读取的槽位。
/// @param onRecord 每收到一条即回调。**请在此即时落库**，这样中途断开时已收到的记录不会丢。
/// @param completion 本轮结束时回调。`records` 是本轮去重后的全部记录，
///                   `reason` 说明结束原因，据此判断是否需要重试。
///                   **该 block 在主队列回调**，与其他 block 不同。
- (void)getHistoryForUser:(VTMJMRBPUser)user
                   record:(nullable void(^)(VTMJMRBPRecordModel *record))onRecord
               completion:(nullable void(^)(NSArray<VTMJMRBPRecordModel *> *records,
                                            VTMJMRBPHistoryEndReason reason))completion;

/// @brief 中断正在进行的存储记录读取。
///
/// @discussion 实现方式是停止向设备确认，设备约 3 秒后自行终止上传。
/// 调用后 completion 会立即以 `VTMJMRBPHistoryEndReasonCancelled` 回调。
/// 没有进行中的读取时本方法无副作用。
- (void)cancelHistorySync;

/// @brief 删除设备端全部槽位的存储记录。
///
/// @warning **不可逆，且设备不返回确认。** 由于无法确认上一次读取是否完整，
/// 删除前请确保调用方已经拿到并持久化了需要的记录。测量进行中设备会忽略该指令。
- (void)deleteAllHistory;

#pragma mark - 主动上报

/// @brief 注册实时压力上报。测量开始后设备持续推送袖带压力。
/// @discussion 只做注册，不发送任何指令。
- (void)receiveRealtimePressure:(void(^)(VTMJMRBPPressureModel *pressure))block;

/// @brief 注册实时测量结果上报。一次测量正常结束时回调。
/// @discussion 只做注册，不发送任何指令。
/// 设备存储的历史记录不走这里，走 `-getHistoryForUser:record:completion:`。
- (void)receiveMeasurementResult:(void(^)(VTMJMRBPRecordModel *record))block;

/// @brief 注册故障上报。测量失败时设备主动上报故障码。
///
/// @discussion 只做注册，不发送任何指令。回调给出的是**设备上报的原始码值**，
/// 库不附带文案。码值含义见随附的协议对照表，文案与本地化由调用方处理。
- (void)receiveFaultCode:(void(^)(NSUInteger code))block;

#pragma mark - 实时结果的确认

/// @brief 是否由 SDK 自动确认实时测量结果。默认 `YES`。
///
/// @discussion **确认在设备侧等于「这条你可以忘了」。** 设备收到确认后会把该条记录
/// 标记为已上传：不再重发，也**不会**转存为可通过
/// `-getHistoryForUser:record:completion:` 再次读取的存储记录。
///
/// 因此当调用方是在收到回调之后才落库、而落库有失败的可能时（写库失败、进程被杀、
/// 上传服务端失败），自动确认相当于 SDK 替调用方做出了一个它无法保证的承诺，
/// 一旦落库没成功，这条测量数据就再也拿不回来了。这种场景请置为 `NO`，
/// 在数据确实落库之后调用 `-acknowledgeMeasurementResult`。
///
/// 置为 `NO` 且始终不确认时，设备会每隔约 1 秒重发同一条结果、共 3 次
/// —— 也就是 `-receiveMeasurementResult:` 注册的 block 会被调用 3 次，
/// **调用方需要自行按 `user` + `date` 去重** —— 之后设备把它转存为存储记录。
///
/// @warning 该开关**只作用于实时测量结果**。读取存储记录时的确认是无条件发送的，
/// 因为设备靠它推进到下一条，不确认会让这一轮读取停在原地。
@property (nonatomic, assign) BOOL acknowledgesMeasurementResult;

/// @brief 确认当前待确认的实时测量结果。
///
/// @discussion 供 `acknowledgesMeasurementResult` 为 `NO` 时使用：在数据确实落库之后
/// 调用，设备收到后才会把该条标记为已上传。
///
/// 设备的重发窗口约 3 秒，超时后它会自行把记录转存为存储记录；此时再调用本方法
/// 不会有效果，也不会有副作用。没有待确认的结果时调用本方法同样无副作用。
///
/// @warning 读取存储记录期间本方法会被拒绝并记一条警告。确认报文不带任何标识，
/// 设备只会把它算在「当前正在等待确认的那条」上，此时插入确认会让设备提前推进队列、
/// 跳过一条记录。
- (void)acknowledgeMeasurementResult;

@end

NS_ASSUME_NONNULL_END
