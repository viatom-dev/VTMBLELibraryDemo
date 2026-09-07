//
//  VTMDemoCentralManager.m
//  VTMBLEDemo
//

#import "VTMDemoCentralManager.h"

NSString *VTMDemoDeviceKindName(VTMDemoDeviceKind kind) {
    switch (kind) {
        case VTMDemoDeviceKindWBP02: return @"WBP02";
        case VTMDemoDeviceKindPM10:  return @"PM10";
    }
    return @"Unknown";
}


#pragma mark -

@interface VTMDemoDiscovery ()
@property (nonatomic, strong) CBPeripheral *peripheral;
@property (nonatomic, copy) NSString *displayName;
@property (nonatomic, strong) NSNumber *rssi;
@end

@implementation VTMDemoDiscovery
@end


#pragma mark -

@interface VTMDemoCentralManager () <CBCentralManagerDelegate>

@property (nonatomic, strong) CBCentralManager *central;
@property (nonatomic, strong) NSMutableArray <VTMDemoDiscovery *> *discoveryArrM;

@end


@implementation VTMDemoCentralManager

- (instancetype)init {
    self = [super init];
    if (self) {
        // 把回调队列固定成主队列。见头文件里关于线程的说明。
        _central = [[CBCentralManager alloc] initWithDelegate:self
                                                       queue:dispatch_get_main_queue()
                                                     options:nil];
    }
    return self;
}

- (NSMutableArray <VTMDemoDiscovery *> *)discoveryArrM {
    if (!_discoveryArrM) {
        _discoveryArrM = [NSMutableArray arrayWithCapacity:20];
    }
    return _discoveryArrM;
}

#pragma mark - 对外

- (CBManagerState)state {
    return self.central.state;
}

- (BOOL)isScanning {
    return self.central.isScanning;
}

- (NSArray <VTMDemoDiscovery *> *)discoveries {
    return [self.discoveryArrM copy];
}

- (void)startScan {
    if (self.central.state != CBManagerStatePoweredOn) {
        return;
    }
    if (self.central.isScanning) {
        return;
    }

    // 这里传 nil 表示扫描所有外设，然后按名字过滤。
    //
    // 生产环境建议改成按 service UUID 扫描：
    //   [self.central scanForPeripheralsWithServices:@[serviceUUID] options:nil];
    // 只有指定了 service UUID，App 退到后台后系统才会继续投递扫描结果。
    // 传 nil 在后台会拿不到任何回调。
    [self.central scanForPeripheralsWithServices:nil
                                        options:@{CBCentralManagerScanOptionAllowDuplicatesKey: @NO}];
}

- (void)stopScan {
    [self.central stopScan];
}

- (void)clearDiscoveries {
    [self.discoveryArrM removeAllObjects];
    [self notifyDiscoveriesChanged];
}

- (void)connectPeripheral:(CBPeripheral *)peripheral {
    // 连接前先停扫描。边扫边连会明显拉长连接耗时。
    [self stopScan];
    [self.central connectPeripheral:peripheral options:nil];
}

- (void)disconnectPeripheral:(CBPeripheral *)peripheral {
    [self.central cancelPeripheralConnection:peripheral];
}

#pragma mark - CBCentralManagerDelegate

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    if (central.state != CBManagerStatePoweredOn) {
        [self.discoveryArrM removeAllObjects];
        [self notifyDiscoveriesChanged];
    }
    if ([self.delegate respondsToSelector:@selector(centralManager:didUpdateState:)]) {
        [self.delegate centralManager:self didUpdateState:central.state];
    }
}

- (void)centralManager:(CBCentralManager *)central
 didDiscoverPeripheral:(CBPeripheral *)peripheral
     advertisementData:(NSDictionary<NSString *,id> *)advertisementData
                  RSSI:(NSNumber *)RSSI {

    NSString *name = advertisementData[CBAdvertisementDataLocalNameKey] ?: peripheral.name;
    if (name.length == 0) {
        // 没有名字的外设对 Demo 没有意义，直接丢掉，否则列表会被环境里的
        // 各种 BLE 设备刷满。
        return;
    }

    if (self.nameFilter.length > 0 &&
        [name rangeOfString:self.nameFilter options:NSCaseInsensitiveSearch].location == NSNotFound) {
        return;
    }

    VTMDemoDiscovery *discovery = [self discoveryForPeripheral:peripheral];
    if (!discovery) {
        discovery = [[VTMDemoDiscovery alloc] init];
        discovery.peripheral = peripheral;   // 必须持有，否则外设会被释放
        [self.discoveryArrM addObject:discovery];
    }
    discovery.displayName = name;
    discovery.rssi = RSSI;

    [self.discoveryArrM sortUsingComparator:^NSComparisonResult(VTMDemoDiscovery *a, VTMDemoDiscovery *b) {
        return [b.rssi compare:a.rssi];
    }];

    [self notifyDiscoveriesChanged];
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    // 连上了，但**还不能发指令**。接下来要把这个 peripheral 交给 SDK 的 session，
    // 等 -sessionDeployCompletion: 回调才算真正就绪。见 VTMDemoDeviceViewController。
    if ([self.delegate respondsToSelector:@selector(centralManager:didConnectPeripheral:)]) {
        [self.delegate centralManager:self didConnectPeripheral:peripheral];
    }
}

- (void)centralManager:(CBCentralManager *)central
didFailToConnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error {
    if ([self.delegate respondsToSelector:@selector(centralManager:didFailToConnectPeripheral:error:)]) {
        [self.delegate centralManager:self didFailToConnectPeripheral:peripheral error:error];
    }
}

- (void)centralManager:(CBCentralManager *)central
didDisconnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error {
    // 断线重连策略由 App 决定，SDK 不介入。Demo 只上报，不自动重连。
    if ([self.delegate respondsToSelector:@selector(centralManager:didDisconnectPeripheral:error:)]) {
        [self.delegate centralManager:self didDisconnectPeripheral:peripheral error:error];
    }
}

#pragma mark - 私有

- (nullable VTMDemoDiscovery *)discoveryForPeripheral:(CBPeripheral *)peripheral {
    for (VTMDemoDiscovery *discovery in self.discoveryArrM) {
        if ([discovery.peripheral isEqual:peripheral]) {
            return discovery;
        }
    }
    return nil;
}

- (void)notifyDiscoveriesChanged {
    if ([self.delegate respondsToSelector:@selector(centralManagerDidUpdateDiscoveries:)]) {
        [self.delegate centralManagerDidUpdateDiscoveries:self];
    }
}

@end
