//
//  VTMDemoDeviceViewController.h
//  VTMBLEDemo
//

#import "VTMDemoActionListViewController.h"
#import "VTMDemoCentralManager.h"

#import <VTMBLELibrary/VTMBLELibrary.h>

NS_ASSUME_NONNULL_BEGIN

/// 两个设备页共用的骨架，负责 session 的生命周期。
///
/// 集成 VTMBLELibrary 的完整顺序：
///
/// 1. 用自己的 `CBCentralManager` 扫描并连接 —— 见 `VTMDemoCentralManager`
/// 2. `session = [VTMXxxBLESession session]`
/// 3. `session.sessionDelegate = self`
/// 4. `session.peripheral = 已连接的 peripheral`  ← 赋值即开始发现服务与特征值
/// 5. 等 `-sessionDeployCompletion:` 回调
/// 6. 才可以开始发指令
///
/// 第 4 步到第 5 步之间发出的指令会丢，因为 tx 特征值还没找到。
@interface VTMDemoDeviceViewController : VTMDemoActionListViewController <VTMBLECoreSessionDelegate>

- (instancetype)initWithPeripheral:(CBPeripheral *)peripheral
                    centralManager:(VTMDemoCentralManager *)centralManager;

@property (nonatomic, strong, readonly) CBPeripheral *peripheral;
@property (nonatomic, strong, readonly) VTMDemoCentralManager *centralManager;

/// 由 `-makeSession` 创建。子类通过自己的 typed 属性访问具体类型。
@property (nonatomic, strong, readonly) VTMBLECoreSession *session;

/// 部署是否已完成。未完成时列表不可点。
@property (nonatomic, readonly) BOOL deployed;

#pragma mark - 子类覆写

/// 创建具体设备的 session。注意 `VTMBLECoreSession` 的 `-init` / `+new` 是
/// `NS_UNAVAILABLE`，必须走设备类的 `+session`。
- (VTMBLECoreSession *)makeSession;

/// `-sessionDeployCompletion:` 之后调用。适合在这里注册设备主动上报的 block。
- (void)sessionDidDeploy;

@end

NS_ASSUME_NONNULL_END
