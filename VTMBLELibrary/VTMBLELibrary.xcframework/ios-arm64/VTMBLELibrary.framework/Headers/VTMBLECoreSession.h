//
//  VTMBLECoreSession.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/13.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@class VTMBLECoreSession;

NS_ASSUME_NONNULL_BEGIN

@protocol VTMBLECoreSessionDelegate <NSObject>

/// @brief rx / tx 特征值都已发现并订阅，会话就绪。
///
/// Called once the rx / tx characteristics have been discovered and the
/// session is ready to use.
///
/// @discussion 在本回调到达之前不要发送任何指令 —— 此时 tx 特征值还是 nil，
/// 写入会被丢弃（库内会记一条 `Error` 级日志）。
///
/// Do not send any command before this callback arrives. Until then the tx
/// characteristic is still nil and writes are dropped (the library records an
/// `Error` level log entry for each one).
///
/// 本回调在 `CBCentralManager` 所在队列上执行，不保证是主队列。
///
/// The callback runs on the queue you handed to `CBCentralManager`, which is
/// not guaranteed to be the main queue.
- (void)sessionDeployCompletion:(VTMBLECoreSession *)session;

@end


/// @brief 所有设备 Session 的基类。
///
/// Base class of every device session.
///
/// @discussion 本库只负责**已连接外设之上的协议层**：指令组包与排队、按 MTU 分片发送、
/// 应答数据的拆帧与解析。`CBCentralManager` 的生命周期不由本库管理 —— 扫描、连接、
/// 重连与断线处理都由宿主 App 负责，连接成功后把 `CBPeripheral` 赋给 `peripheral`
/// 即触发服务与特征值发现，等 `-sessionDeployCompletion:` 回来之后再发指令。
///
/// The library only covers the **protocol layer above an already connected
/// peripheral**: framing and queueing commands, splitting writes by MTU, and
/// decoding responses into model objects. It does not own the
/// `CBCentralManager` lifecycle — scanning, connecting, reconnecting and
/// disconnect handling stay in the host app. Assign the connected
/// `CBPeripheral` to `peripheral` to kick off discovery, then wait for
/// `-sessionDeployCompletion:` before issuing commands.
///
/// @warning 指令必须逐条发送。上一条的回调到达之前不要发下一条 —— 库内的指令队列是
/// 串行的，同一时刻只有队首指令在等应答，并发调用会导致应答与指令错配。
///
/// Commands must be issued one at a time. Do not start the next one before the
/// previous callback arrives: the internal command queue is serial, only the
/// head command is waiting for a response, and concurrent calls will pair
/// responses with the wrong command.
@interface VTMBLECoreSession : NSObject

/// @brief 不可用。请用设备自己的 `+session` 工厂方法创建，例如
/// `+[VTMWBP02BLESession session]`。
///
/// Unavailable. Create a session through the device specific `+session`
/// factory, for example `+[VTMWBP02BLESession session]`.
///
/// @discussion Session 必须绑定设备描述对象才知道自己的 rx / tx 特征值，
/// 裸 `-init` 只会造出一个不可用的实例。
///
/// A session must be bound to a device description object to know its rx / tx
/// characteristics, so a plain `-init` would only produce an unusable instance.
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/// @brief 接收服务与特征值的部署结果。/ Receives the deployment result of services and characteristics.
@property (nonatomic, weak, nullable) id <VTMBLECoreSessionDelegate> sessionDelegate;

/// @brief 已连接的外设。未绑定时为 `nil`。
///
/// The connected peripheral. `nil` until you attach one.
///
/// @discussion 赋值会清空指令队列与收包缓存、丢弃已匹配的特征值引用，
/// 然后重新开始服务与特征值发现。断线重连后重新赋值即可，无需新建 Session。
///
/// Assigning a peripheral clears the command queue and the receive buffer,
/// drops the previously matched characteristics, and restarts service and
/// characteristic discovery. After a reconnect just assign it again; there is
/// no need to create a new session.
///
/// @warning 这是一个带副作用的 setter：赋值即启动一套异步流程。已知的 API 设计缺陷，
/// 会在下一个大版本改为具名方法。
///
/// This setter has side effects: assigning it starts an asynchronous flow. A
/// known API design flaw, to be replaced by an explicit method in the next
/// major version.
@property (nonatomic, strong, nullable) CBPeripheral *peripheral;

@end

NS_ASSUME_NONNULL_END
