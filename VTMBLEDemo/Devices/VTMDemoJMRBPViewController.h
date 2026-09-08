//
//  VTMDemoJMRBPViewController.h
//  VTMBLEDemo
//

#import "VTMDemoDeviceViewController.h"

NS_ASSUME_NONNULL_BEGIN

/// JMRBP 蓝牙血压计。演示该设备的全部对外 API，含存储记录的幂等全量同步写法。
///
/// JMRBP Bluetooth blood pressure monitor. Demonstrates every public API of this
/// device, including the idempotent full sync for stored records.
@interface VTMDemoJMRBPViewController : VTMDemoDeviceViewController
@end

NS_ASSUME_NONNULL_END
