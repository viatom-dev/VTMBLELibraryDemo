//
//  VTMDemoWBP02ViewController.m
//  VTMBLEDemo
//

#import "VTMDemoWBP02ViewController.h"

@interface VTMDemoWBP02ViewController ()

/// 与 `super.session` 是同一个对象，这里只是省掉每次强转。
@property (nonatomic, weak) VTMWBP02BLESession *wbp02Session;

@property (nonatomic, assign) BOOL handshaked;

@end

@implementation VTMDemoWBP02ViewController

- (void)viewDidLoad {
    self.title = @"WBP02";
    [super viewDidLoad];
}

- (VTMBLECoreSession *)makeSession {
    // `+session` 每次返回一个新实例，虽然名字读起来像单例。
    // 保留这个名字是因为 Swift 会把它映射成 `VTMWBP02BLESession()`。
    VTMWBP02BLESession *session = [VTMWBP02BLESession session];
    self.wbp02Session = session;
    return session;
}

- (void)sessionDidDeploy {
    // 主动上报类的接口（实时袖带压、测量结束）只是**注册一个长期持有的 block**，
    // 不发任何指令，所以要尽早注册，否则设备推上来的数据没人接。
    [self subscribeRealtimeData];
    [self subscribeMeasurementEnd];

    [self log:@"已注册实时数据与测量结束回调"];
    [self log:@"下一步：先执行「握手」，其余指令都依赖握手完成"];
}

#pragma mark - 指令列表

- (NSArray <VTMDemoActionSection *> *)buildSections {
    __weak typeof(self) weakSelf = self;

    VTMDemoActionSection *handshake = [VTMDemoActionSection
        sectionWithTitle:@"1. 握手"
                  footer:@"握手是一条链式流程：SDK 内部会依次发 握手 → 同步时间 → 设置用户 → 读设备信息，"
                          "只有最后一步的应答才会触发 callback。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"握手并读取设备 SN"
                               detail:@"-handshakes:"
                              handler:^{ [weakSelf doHandshake]; }],
    ]];

    VTMDemoActionSection *query = [VTMDemoActionSection
        sectionWithTitle:@"2. 读取"
                  footer:@"指令必须逐条发送。SDK 内部有队列，但请等 callback 回来再点下一条，"
                          "不要并发调用。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"电池电量"
                               detail:@"-getBatteyInfo:"
                              handler:^{ [weakSelf doGetBattery]; }],
        [VTMDemoAction actionWithTitle:@"历史血压记录（今天）"
                               detail:@"-getHistory:callback:"
                              handler:^{ [weakSelf doGetHistoryForDate:NSDate.date]; }],
        [VTMDemoAction actionWithTitle:@"自动测量设置"
                               detail:@"-getAutoModeSetiingInfo:"
                              handler:^{ [weakSelf doGetAutoModeSetting]; }],
    ]];

    VTMDemoActionSection *program = [VTMDemoActionSection
        sectionWithTitle:@"3. 编程（写入设备）"
                  footer:@"会真的改设备设置，请在测试机上操作。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"开启白天自动测量"
                               detail:@"-setDaylightAutoMode:callback:"
                              handler:^{ [weakSelf doSetDaylightAutoMode:YES]; }],
        [VTMDemoAction actionWithTitle:@"关闭白天自动测量"
                               detail:@"-setDaylightAutoMode:callback:"
                              handler:^{ [weakSelf doSetDaylightAutoMode:NO]; }],
        [VTMDemoAction actionWithTitle:@"白天时段 08:00–22:00 / 30 分钟"
                               detail:@"-setDaylightAutoStartTime:endTime:timeInterval:callback:"
                              handler:^{ [weakSelf doSetDaylightWindow]; }],
        [VTMDemoAction actionWithTitle:@"开启夜间自动测量"
                               detail:@"-setNightlyAutoMode:callback:"
                              handler:^{ [weakSelf doSetNightlyAutoMode:YES]; }],
        [VTMDemoAction actionWithTitle:@"关闭夜间自动测量"
                               detail:@"-setNightlyAutoMode:callback:"
                              handler:^{ [weakSelf doSetNightlyAutoMode:NO]; }],
        [VTMDemoAction actionWithTitle:@"夜间时段 22:00–08:00 / 60 分钟"
                               detail:@"-setNightlyAutoStartTime:endTime:timeInterval:callback:"
                              handler:^{ [weakSelf doSetNightlyWindow]; }],
    ]];

    return @[handshake, query, program];
}

#pragma mark - 指令实现

- (void)doHandshake {
    [self log:@"→ 握手…"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session handshakes:^(NSString *deviceSN) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        self.handshaked = YES;
        [self log:@"← 握手完成，SN = %@", deviceSN.length ? deviceSN : @"(空)"];
    }];
}

- (void)doGetBattery {
    if (![self requireHandshake]) { return; }
    [self log:@"→ 读取电池电量…"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session getBatteyInfo:^(VTMWBP02BatteryModel *model) {
        [weakSelf log:@"← 电量 %ld%%，充电中 %@，已充满 %@",
             (long)model.percent,
             model.isCharging ? @"是" : @"否",
             model.isFull ? @"是" : @"否"];
    }];
}

- (void)doGetHistoryForDate:(NSDate *)date {
    if (![self requireHandshake]) { return; }
    [self log:@"→ 读取历史记录…"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session getHistory:date callback:^(NSArray *results) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        [self log:@"← 共 %lu 条记录", (unsigned long)results.count];

        // callback 的元素类型是 NSArray（未泛型化），实际元素是 VTMWBP02DataModel。
        // 这里做一次类型判断，避免设备返回意外内容时崩掉。
        NSUInteger index = 0;
        for (id item in results) {
            if (![item isKindOfClass:VTMWBP02DataModel.class]) {
                [self log:@"   [%lu] 非预期类型 %@", (unsigned long)index++, NSStringFromClass([item class])];
                continue;
            }
            VTMWBP02DataModel *record = item;
            [self log:@"   [%lu] %@  %u/%u mmHg  %u bpm  err=%u",
                 (unsigned long)index++, record.date,
                 record.sys, record.dia, record.rate, record.errorCode];
        }
    }];
}

- (void)doGetAutoModeSetting {
    if (![self requireHandshake]) { return; }
    [self log:@"→ 读取自动测量设置…"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session getAutoModeSetiingInfo:^(VTMWBP02AutoModeState *autoState) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        // 注意 `daylightEntTime` / `nightlyEntTime` 是已发布 API 里的拼写错误
        // （End 写成了 Ent），不能改名，否则集成方会编译不过。
        [self log:@"← 白天 %@  %@–%@  间隔 %ld 分钟",
             autoState.daylightMode ? @"开" : @"关",
             autoState.daylightStartTime, autoState.daylightEntTime,
             (long)autoState.daylightInterval];
        [self log:@"  夜间 %@  %@–%@  间隔 %ld 分钟",
             autoState.nightlyMode ? @"开" : @"关",
             autoState.nightlyStartTime, autoState.nightlyEntTime,
             (long)autoState.nightlyInterval];
    }];
}

- (void)doSetDaylightAutoMode:(BOOL)on {
    if (![self requireHandshake]) { return; }
    [self log:@"→ %@白天自动测量…", on ? @"开启" : @"关闭"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session setDaylightAutoMode:on callback:^(NSNumber *autoMode) {
        [weakSelf log:@"← 白天自动测量 = %@", autoMode];
    }];
}

- (void)doSetNightlyAutoMode:(BOOL)on {
    if (![self requireHandshake]) { return; }
    [self log:@"→ %@夜间自动测量…", on ? @"开启" : @"关闭"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session setNightlyAutoMode:on callback:^(NSNumber *autoMode) {
        [weakSelf log:@"← 夜间自动测量 = %@", autoMode];
    }];
}

- (void)doSetDaylightWindow {
    if (![self requireHandshake]) { return; }
    // 时间字符串格式是 "HH:mm"，SDK 内部按 ":" 拆分。
    [self log:@"→ 设置白天时段 08:00–22:00，间隔 30 分钟…"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session setDaylightAutoStartTime:@"08:00"
                                       endTime:@"22:00"
                                  timeInterval:30
                                      callback:^(NSNumber *success) {
        [weakSelf log:@"← 白天时段写入结果 = %@", success];
    }];
}

- (void)doSetNightlyWindow {
    if (![self requireHandshake]) { return; }
    [self log:@"→ 设置夜间时段 22:00–08:00，间隔 60 分钟…"];

    __weak typeof(self) weakSelf = self;
    [self.wbp02Session setNightlyAutoStartTime:@"22:00"
                                       endTime:@"08:00"
                                  timeInterval:60
                                      callback:^(NSNumber *success) {
        [weakSelf log:@"← 夜间时段写入结果 = %@", success];
    }];
}

#pragma mark - 主动上报

- (void)subscribeRealtimeData {
    __weak typeof(self) weakSelf = self;
    [self.wbp02Session receiveRealtimeData:^(NSNumber *pressure, VTMWBP02DataModel *result) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        // ⚠️ 这两个参数在头文件里声明为 nonnull，但实际实现会给其中一个传 nil：
        // 打气过程中回调 (压力值, nil)，测量结束时回调 (nil, 结果模型)。
        // 所以两个参数都必须判空，Swift 侧尤其要注意 —— 非可选参数收到 nil 会直接崩。
        if (pressure != nil) {
            [self log:@"← 实时袖带压 %@ mmHg", pressure];
        }
        if (result != nil) {
            [self log:@"← 测量结果 %u/%u mmHg  %u bpm  err=%u  %@",
                 result.sys, result.dia, result.rate, result.errorCode, result.date];
        }
    }];
}

- (void)subscribeMeasurementEnd {
    __weak typeof(self) weakSelf = self;
    [self.wbp02Session monitorPressureEnd:^(BOOL end) {
        [weakSelf log:@"← 测量结束上报 end=%@", end ? @"YES" : @"NO"];
    }];
}

#pragma mark - 私有

- (BOOL)requireHandshake {
    if (self.handshaked) {
        return YES;
    }
    [self log:@"请先执行「握手」。未握手时设备不会响应其他指令。"];
    return NO;
}

@end
