//
//  VTMDemoCentralManager.h
//  VTMBLEDemo
//
//  扫描与连接。这一层 **不属于 VTMBLELibrary 的职责**。
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

NS_ASSUME_NONNULL_BEGIN

/// Demo 支持的设备型号。决定连接成功后创建哪一个 session 子类。
typedef NS_ENUM(NSInteger, VTMDemoDeviceKind) {
    VTMDemoDeviceKindWBP02,
    VTMDemoDeviceKindPM10,
    VTMDemoDeviceKindJMRBP,
};

/// 型号总数。扫描页的分段控件按它生成，新增型号不用再改扫描页。
FOUNDATION_EXPORT const NSInteger VTMDemoDeviceKindCount;

FOUNDATION_EXPORT NSString *VTMDemoDeviceKindName(VTMDemoDeviceKind kind);


/// 一次扫描发现的结果。
@interface VTMDemoDiscovery : NSObject

@property (nonatomic, strong, readonly) CBPeripheral *peripheral;
/// 广播包里的 local name，通常比 `peripheral.name` 更早可用。
@property (nonatomic, copy, readonly) NSString *displayName;
@property (nonatomic, strong, readonly) NSNumber *rssi;

@end


@class VTMDemoCentralManager;

@protocol VTMDemoCentralManagerDelegate <NSObject>

@optional
- (void)centralManager:(VTMDemoCentralManager *)manager didUpdateState:(CBManagerState)state;
- (void)centralManagerDidUpdateDiscoveries:(VTMDemoCentralManager *)manager;
- (void)centralManager:(VTMDemoCentralManager *)manager didConnectPeripheral:(CBPeripheral *)peripheral;
- (void)centralManager:(VTMDemoCentralManager *)manager
didFailToConnectPeripheral:(CBPeripheral *)peripheral
                 error:(nullable NSError *)error;
- (void)centralManager:(VTMDemoCentralManager *)manager
didDisconnectPeripheral:(CBPeripheral *)peripheral
                 error:(nullable NSError *)error;

@end


/// @brief 对 `CBCentralManager` 的一层薄封装。
///
/// @discussion **这个类演示的是 VTMBLELibrary 不负责的那一段。** SDK 只处理「已经连上的
/// 外设之上的协议层」，扫描、连接、断线重连全部由宿主 App 自己管理。集成时最容易卡住的
/// 就是这里，所以 Demo 把它单独拆成一个类。
///
/// 两个关键约定：
///
/// 1. **回调队列。** central manager 用 `dispatch_get_main_queue()` 创建，因此所有
///    CoreBluetooth 回调都在主线程。Demo 也在主线程调用 SDK 的全部 API。SDK 内部对
///    `cmdArr` / `dataPool` 没有加锁，跨队列调用会构成数据竞争，所以请把「调用 SDK」
///    和「CoreBluetooth 回调」固定在同一个队列上。
///
/// 2. **持有外设。** `CBCentralManager` 不会替你 retain `CBPeripheral`，发现到的外设
///    如果没人持有会被释放，连接随即失败。`discoveries` 数组就是为此存在的。
@interface VTMDemoCentralManager : NSObject

@property (nonatomic, weak, nullable) id <VTMDemoCentralManagerDelegate> delegate;

@property (nonatomic, readonly) CBManagerState state;
@property (nonatomic, readonly, getter=isScanning) BOOL scanning;

/// 已发现的外设，按信号强度降序。
@property (nonatomic, copy, readonly) NSArray <VTMDemoDiscovery *> *discoveries;

/// 只保留 `displayName` 含该子串的外设，大小写不敏感。`nil` 或空串表示不过滤。
@property (nonatomic, copy, nullable) NSString *nameFilter;

- (void)startScan;
- (void)stopScan;
- (void)clearDiscoveries;

- (void)connectPeripheral:(CBPeripheral *)peripheral;
- (void)disconnectPeripheral:(CBPeripheral *)peripheral;

@end

NS_ASSUME_NONNULL_END
