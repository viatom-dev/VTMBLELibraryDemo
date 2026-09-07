//
//  VTMDemoDeviceViewController.m
//  VTMBLEDemo
//

#import "VTMDemoDeviceViewController.h"

@interface VTMDemoDeviceViewController ()

@property (nonatomic, strong) CBPeripheral *peripheral;
@property (nonatomic, strong) VTMDemoCentralManager *centralManager;
@property (nonatomic, strong) VTMBLECoreSession *session;
@property (nonatomic, assign) BOOL deployed;

@end

@implementation VTMDemoDeviceViewController

- (instancetype)initWithPeripheral:(CBPeripheral *)peripheral
                    centralManager:(VTMDemoCentralManager *)centralManager {
    self = [super init];
    if (self) {
        _peripheral = peripheral;
        _centralManager = centralManager;
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    self.navigationItem.rightBarButtonItem =
        [[UIBarButtonItem alloc] initWithTitle:@"断开"
                                        style:UIBarButtonItemStylePlain
                                       target:self
                                       action:@selector(disconnect)];

    // 部署完成前不许发指令。
    self.actionsEnabled = NO;

    self.session = [self makeSession];
    self.session.sessionDelegate = self;

    [self log:@"已连接 %@", self.peripheral.name ?: self.peripheral.identifier.UUIDString];
    [self log:@"正在发现服务与特征值…"];

    // 注意这是一个**带副作用的 setter**：赋值会清空 session 状态并启动
    // discoverServices。API 设计上更合适的形态是一个具名方法（如 -attachPeripheral:），
    // 但现阶段对外契约就是这个属性。
    self.session.peripheral = self.peripheral;
}

#pragma mark - 子类覆写

- (VTMBLECoreSession *)makeSession {
    NSAssert(NO, @"%@ 必须覆写 %@", NSStringFromClass(self.class), NSStringFromSelector(_cmd));
    return nil;
}

- (void)sessionDidDeploy {
    // 默认什么都不做。
}

#pragma mark - VTMBLECoreSessionDelegate

- (void)sessionDeployCompletion:(VTMBLECoreSession *)session {
    // 这个回调来自 CoreBluetooth 的回调队列。Demo 把 central manager 建在主队列上，
    // 所以这里就是主线程，可以直接动 UI。如果你把 central manager 建在自定义队列上，
    // 记得自己切回主线程。
    self.deployed = YES;
    self.actionsEnabled = YES;

    [self log:@"session 部署完成，可以开始发指令"];
    [self sessionDidDeploy];
}

#pragma mark - 动作

- (void)disconnect {
    [self.centralManager disconnectPeripheral:self.peripheral];
    [self.navigationController popViewControllerAnimated:YES];
}

#pragma mark - 工具

- (void)dealloc {
    // 页面销毁时摘掉两条回调链路，避免已经离开的页面还在收数据。
    //
    // 这里刻意**不写** `_session.peripheral = nil`：那个 setter 带副作用，
    // 赋 nil 会再跑一遍部署流程（清空数据池、把 mtu 设成 0），并留下一条
    // "attach peripheral: (null)" 的误导日志。
    _session.sessionDelegate = nil;
    if (_peripheral.delegate == (id)_session) {
        _peripheral.delegate = nil;
    }
}

@end
