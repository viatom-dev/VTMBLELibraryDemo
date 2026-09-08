//
//  VTMDemoJMRBPViewController.m
//  VTMBLEDemo
//

#import "VTMDemoJMRBPViewController.h"

@interface VTMDemoJMRBPViewController ()

/// 与 `super.session` 是同一个对象，这里只是省掉每次强转。
@property (nonatomic, weak) VTMJMRBPBLESession *jmrbpSession;

/// 实测项②：本轮结束后自动再发一次读取请求。
@property (nonatomic, assign) BOOL probeRepeatAfterFinish;
/// 实测项③：收到指定条数后主动中断，用来观察设备是断点续传还是从头重发。
@property (nonatomic, assign) NSUInteger probeAbortAfterCount;

/// 自动确认开关在本页的镜像值，与 SDK 的默认值一致。
///
/// 基类的 `-viewDidLoad` 先调 `-reloadActions`（进而 `-buildSections`）、后调 `-makeSession`，
/// 所以构建列表时 session 还不存在，标题不能直接去读 session 的状态。
@property (nonatomic, assign) BOOL autoAcknowledge;

@end

@implementation VTMDemoJMRBPViewController

- (void)viewDidLoad {
    self.title = @"JMRBP";
    self.autoAcknowledge = YES;   // 与 SDK 的默认值保持一致
    [super viewDidLoad];
}

- (VTMBLECoreSession *)makeSession {
    VTMJMRBPBLESession *session = [VTMJMRBPBLESession session];
    session.acknowledgesMeasurementResult = self.autoAcknowledge;
    self.jmrbpSession = session;
    return session;
}

- (void)sessionDidDeploy {
    // 主动上报类接口只是注册一个长期持有的 block，不发任何指令，
    // 所以要在这里尽早注册，否则设备推上来的压力值和测量结果没人接。
    [self subscribePressure];
    [self subscribeResult];
    [self subscribeFault];

    [self log:@"已注册压力 / 结果 / 故障上报"];
    [self log:@"该设备无握手流程，部署完成即可发指令"];
    [self log:@"建议先执行「同步时间」——记录里的时间戳由设备时钟给出"];
    [self log:@"自动确认当前为「%@」，见第 4 组", self.autoAcknowledge ? @"开" : @"关"];
}

#pragma mark - 指令列表

- (NSArray <VTMDemoActionSection *> *)buildSections {
    __weak typeof(self) weakSelf = self;

    VTMDemoActionSection *prepare = [VTMDemoActionSection
        sectionWithTitle:@"1. 准备"
                  footer:@"设备时钟决定记录里的时间戳，断电后可能归零。读取记录前先校时。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"同步时间（当前系统时间）"
                               detail:@"-syncTime:"
                              handler:^{ [weakSelf doSyncTime]; }],
    ]];

    VTMDemoActionSection *control = [VTMDemoActionSection
        sectionWithTitle:@"2. 启停控制"
                  footer:@"⚠️ 启动会让设备对手臂真实加压。协议未定义这几条指令的应答，"
                          "所以方法没有 callback，发出即返回；是否真的动起来看压力上报。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"启动（设备默认配置）"
                               detail:@"-activate"
                              handler:^{ [weakSelf doActivate]; }],
        [VTMDemoAction actionWithTitle:@"启动 · Standard"
                               detail:@"-activateWithProfile: 0x01"
                              handler:^{ [weakSelf doActivateWithProfile:VTMJMRBPProfileStandard name:@"Standard"]; }],
        [VTMDemoAction actionWithTitle:@"启动 · Rhythm"
                               detail:@"-activateWithProfile: 0x02"
                              handler:^{ [weakSelf doActivateWithProfile:VTMJMRBPProfileRhythm name:@"Rhythm"]; }],
        [VTMDemoAction actionWithTitle:@"启动 · Cardiac"
                               detail:@"-activateWithProfile: 0x03"
                              handler:^{ [weakSelf doActivateWithProfile:VTMJMRBPProfileCardiac name:@"Cardiac"]; }],
        [VTMDemoAction actionWithTitle:@"启动 · Averaged"
                               detail:@"-activateWithProfile: 0x04"
                              handler:^{ [weakSelf doActivateWithProfile:VTMJMRBPProfileAveraged name:@"Averaged"]; }],
        [VTMDemoAction actionWithTitle:@"中止"
                               detail:@"-deactivate"
                              handler:^{ [weakSelf doDeactivate]; }],
    ]];

    VTMDemoActionSection *records = [VTMDemoActionSection
        sectionWithTitle:@"3. 存储记录"
                  footer:@"设备不给总条数也不给结束标志，SDK 靠静默超时收尾，"
                          "所以 completion 的 reason 是 Idle 时**不代表已拉全**。"
                          "正确用法是每次连上都无条件拉一次，按 槽位 + 时间 去重合并。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"读取 User1"
                               detail:@"-getHistoryForUser:record:completion:"
                              handler:^{ [weakSelf doGetHistoryForUser:VTMJMRBPUser1 name:@"User1"]; }],
        [VTMDemoAction actionWithTitle:@"读取 User2"
                               detail:@"-getHistoryForUser:record:completion:"
                              handler:^{ [weakSelf doGetHistoryForUser:VTMJMRBPUser2 name:@"User2"]; }],
        [VTMDemoAction actionWithTitle:@"读取 User3"
                               detail:@"文档未描述，实测设备是否支持"
                              handler:^{ [weakSelf doGetHistoryForUser:VTMJMRBPUser3 name:@"User3"]; }],
        [VTMDemoAction actionWithTitle:@"读取 User4"
                               detail:@"文档未描述，实测设备是否支持"
                              handler:^{ [weakSelf doGetHistoryForUser:VTMJMRBPUser4 name:@"User4"]; }],
        [VTMDemoAction actionWithTitle:@"读取 全部槽位"
                               detail:@"槽位参数 0x64，取自厂商 SDK，待实测"
                              handler:^{ [weakSelf doGetHistoryForUser:VTMJMRBPUserAll name:@"All"]; }],
        [VTMDemoAction actionWithTitle:@"取消读取"
                               detail:@"-cancelHistorySync（停止回 ACK，设备自行终止）"
                              handler:^{ [weakSelf doCancel]; }],
        [VTMDemoAction actionWithTitle:@"⚠️ 删除设备全部记录"
                               detail:@"-deleteAllHistory（不可逆，无应答）"
                              handler:^{ [weakSelf confirmDeleteAll]; }],
    ]];

    VTMDemoActionSection *ack = [VTMDemoActionSection
        sectionWithTitle:@"4. 实时结果的确认"
                  footer:@"厂商已确认：确认报文在设备侧等于「标记为已上传」——该条记录此后"
                          "既不重发，也不会转存为可再次读取的存储记录，这条数据再也拿不回来。"
                          "需要「落库成功后才确认」的调用方应关掉自动确认，落库后手动确认。"
                 actions:@[
        [VTMDemoAction actionWithTitle:[NSString stringWithFormat:@"自动确认：%@（点击切换）",
                                        self.autoAcknowledge ? @"开" : @"关"]
                               detail:@"acknowledgesMeasurementResult"
                              handler:^{ [weakSelf toggleAutoAcknowledge]; }],
        [VTMDemoAction actionWithTitle:@"手动确认一条实时结果"
                               detail:@"-acknowledgeMeasurementResult"
                              handler:^{ [weakSelf doAcknowledge]; }],
    ]];

    VTMDemoActionSection *probe = [VTMDemoActionSection
        sectionWithTitle:@"5. 待实测项"
                  footer:@"这几项是协议文档没写清、需要真机确认的点。⚠️ 当前样机对读取存储记录"
                          "的指令零响应（厂商 demo 同样读不到），已在向厂商确认，"
                          "在此之前下面这几项都跑不出结论。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"① 记录到达顺序（新→旧？）"
                               detail:@"按到达顺序打印时间戳并判断单调性"
                              handler:^{ [weakSelf probeOrdering]; }],
        [VTMDemoAction actionWithTitle:@"② 确认后是否不再重复下发"
                               detail:@"本轮结束后立刻再拉一次，看第二次还有没有数据"
                              handler:^{ [weakSelf probeCursor]; }],
        [VTMDemoAction actionWithTitle:@"③ 中断后是断点续传还是从头"
                               detail:@"收到 3 条后中断，再拉一次看首条是第 4 条还是第 1 条"
                              handler:^{ [weakSelf probeResume]; }],
    ]];

    return @[prepare, control, records, ack, probe];
}

#pragma mark - 指令实现

- (void)doSyncTime {
    [self log:@"→ 同步时间 %@", [self nowText]];
    [self.jmrbpSession syncTime:NSDate.date];
    [self log:@"  已发出（协议无应答，无法确认设备是否接受）"];
}

- (void)doActivate {
    [self log:@"→ 启动（设备默认配置）"];
    [self.jmrbpSession activate];
}

- (void)doActivateWithProfile:(VTMJMRBPProfile)profile name:(NSString *)name {
    [self log:@"→ 启动 · %@（0x%02lX）", name, (unsigned long)profile];
    [self.jmrbpSession activateWithProfile:profile];
}

- (void)doDeactivate {
    [self log:@"→ 中止"];
    [self.jmrbpSession deactivate];
}

- (void)doCancel {
    [self log:@"→ 取消读取"];
    [self.jmrbpSession cancelHistorySync];
}

#pragma mark - 实时结果的确认

- (void)toggleAutoAcknowledge {
    BOOL on = !self.autoAcknowledge;
    self.autoAcknowledge = on;
    self.jmrbpSession.acknowledgesMeasurementResult = on;

    if (on) {
        [self log:@"自动确认已开启：SDK 收到实时结果就立刻确认"];
        [self log:@"  ⚠️ 设备随即把该条标记为已上传，落库失败也拿不回来了"];
    } else {
        [self log:@"自动确认已关闭：SDK 不再代为确认"];
        [self log:@"  设备会每隔约 1 秒重发同一条、共 3 次（回调会触发 3 次，需自行去重）"];
        [self log:@"  在这 3 秒窗口内点「手动确认」才会让设备标记为已上传"];
    }
    // 分组标题里带着当前状态，切换后要重建列表。
    [self reloadActions];
}

- (void)doAcknowledge {
    [self log:@"→ 手动确认实时结果"];
    [self.jmrbpSession acknowledgeMeasurementResult];
    [self log:@"  若没有待确认的结果、或正在读取存储记录，SDK 会拒绝并记一条警告"];
}

- (void)confirmDeleteAll {
    UIAlertController *alert = [UIAlertController
        alertControllerWithTitle:@"删除设备全部记录？"
                         message:@"不可逆，且设备不返回确认。由于无法确认上一次读取是否完整，"
                                  "删除后未同步的记录会永久丢失。"
                  preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"取消" style:UIAlertActionStyleCancel handler:nil]];

    __weak typeof(self) weakSelf = self;
    [alert addAction:[UIAlertAction actionWithTitle:@"删除"
                                             style:UIAlertActionStyleDestructive
                                           handler:^(UIAlertAction *action) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }
        [self log:@"→ 删除设备全部记录"];
        [self.jmrbpSession deleteAllHistory];
        [self log:@"  已发出（协议无应答）"];
    }]];
    [self presentViewController:alert animated:YES completion:nil];
}

#pragma mark - 存储记录

- (void)doGetHistoryForUser:(VTMJMRBPUser)user name:(NSString *)name {
    [self log:@"→ 读取 %@ 的存储记录…", name];

    __block NSUInteger index = 0;
    __weak typeof(self) weakSelf = self;

    [self.jmrbpSession getHistoryForUser:user record:^(VTMJMRBPRecordModel *record) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        index++;
        [self logRecord:record prefix:[NSString stringWithFormat:@"  [%lu]", (unsigned long)index]];

        // 实测项③：到指定条数就停止回 ACK，让设备自行终止。
        if (self.probeAbortAfterCount > 0 && index >= self.probeAbortAfterCount) {
            [self log:@"  已收到 %lu 条，主动中断（停止回 ACK）", (unsigned long)index];
            self.probeAbortAfterCount = 0;
            [self.jmrbpSession cancelHistorySync];
        }
    } completion:^(NSArray<VTMJMRBPRecordModel *> *records, VTMJMRBPHistoryEndReason reason) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        [self log:@"← 本轮结束：%lu 条，reason = %@",
             (unsigned long)records.count, [self reasonText:reason]];

        if (reason == VTMJMRBPHistoryEndReasonIdle) {
            [self log:@"  注意 Idle 不等于「已拉全」，未取到的下轮会再出现"];
        }

        if (self.probeRepeatAfterFinish) {
            self.probeRepeatAfterFinish = NO;
            [self log:@"— 实测②：立刻再拉一次同一槽位 —"];
            [self log:@"  若第二次为 0 条 → ACK 就是「标记已上传」，可做增量同步"];
            [self log:@"  若第二次仍是全量 → 只能维持幂等全量方案"];
            [self doGetHistoryForUser:user name:name];
        }
    }];
}

#pragma mark - 待实测项

- (void)probeOrdering {
    [self log:@"— 实测①：记录到达顺序 —"];
    [self log:@"  请确保设备里有 3 条以上时间不同的记录"];

    NSMutableArray <NSString *> *dates = [NSMutableArray array];
    __weak typeof(self) weakSelf = self;

    [self.jmrbpSession getHistoryForUser:VTMJMRBPUser1 record:^(VTMJMRBPRecordModel *record) {
        [dates addObject:record.date ?: @""];
        [weakSelf log:@"  收到 %@", record.date];
    } completion:^(NSArray<VTMJMRBPRecordModel *> *records, VTMJMRBPHistoryEndReason reason) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        [self log:@"← 共 %lu 条，reason = %@",
             (unsigned long)dates.count, [self reasonText:reason]];

        if (dates.count < 2) {
            [self log:@"  少于 2 条，判断不了顺序。请在设备上多测几次再来"];
            return;
        }

        // 时间戳是 yyyy-MM-dd HH:mm:ss 定长格式，字典序等价于时间序。
        BOOL descending = YES, ascending = YES;
        for (NSUInteger i = 1; i < dates.count; i++) {
            NSComparisonResult r = [dates[i - 1] compare:dates[i]];
            if (r == NSOrderedAscending)  { descending = NO; }
            if (r == NSOrderedDescending) { ascending = NO; }
        }

        if (descending && !ascending) {
            [self log:@"  结论：新 → 旧（时间递减）"];
        } else if (ascending && !descending) {
            [self log:@"  结论：旧 → 新（时间递增）"];
        } else if (ascending && descending) {
            [self log:@"  结论：时间戳全部相同，换成时间不同的记录再测"];
        } else {
            [self log:@"  结论：无序。时间不能作为终止依据，只能维持幂等全量"];
        }
    }];
}

- (void)probeCursor {
    [self log:@"— 实测②：ACK 是否等于「标记已上传」—"];
    self.probeRepeatAfterFinish = YES;
    [self doGetHistoryForUser:VTMJMRBPUser1 name:@"User1"];
}

- (void)probeResume {
    [self log:@"— 实测③：中断后是断点续传还是从头 —"];
    [self log:@"  先收 3 条后中断，等约 5 秒设备自行终止，再手动点一次「读取 User1」"];
    [self log:@"  第二次首条是第 4 条 → 断点续传；是第 1 条 → 从头重发"];
    self.probeAbortAfterCount = 3;
    [self doGetHistoryForUser:VTMJMRBPUser1 name:@"User1"];
}

#pragma mark - 主动上报

- (void)subscribePressure {
    __weak typeof(self) weakSelf = self;
    [self.jmrbpSession receiveRealtimePressure:^(VTMJMRBPPressureModel *pressure) {
        // 换算系数是从协议示例反推的，两个值一起打，便于实测核对。
        [weakSelf log:@"← 压力 %.2f mmHg（原始码值 %lu）",
             pressure.mmHg, (unsigned long)pressure.rawValue];
    }];
}

- (void)subscribeResult {
    __weak typeof(self) weakSelf = self;
    [self.jmrbpSession receiveMeasurementResult:^(VTMJMRBPRecordModel *record) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }
        [self log:@"← 实时测量结果"];
        [self logRecord:record prefix:@"  "];
    }];
}

- (void)subscribeFault {
    __weak typeof(self) weakSelf = self;
    [self.jmrbpSession receiveFaultCode:^(NSUInteger code) {
        // SDK 只给原始码值，不带文案。文案与本地化是调用方的事。
        [weakSelf log:@"← 故障码 0x%02lX", (unsigned long)code];
    }];
}

#pragma mark - 私有

- (void)logRecord:(VTMJMRBPRecordModel *)record prefix:(NSString *)prefix {
    [self log:@"%@ %@  %u/%u mmHg  %u bpm  槽位 %lu",
         prefix, record.date, record.sys, record.dia, record.rate, (unsigned long)record.user];
    [self log:@"%@ 房颤 %@  不规则脉搏 %@  历史 %@  dataMode 0x%02X",
         prefix,
         record.isAtrialFibrillation ? @"是" : @"否",
         record.isIrregularPulse ? @"是" : @"否",
         record.isHistory ? @"是" : @"否",
         record.dataMode];
    // 实测项④：这四个字节协议标为预留，厂商 SDK 解释为动脉硬化 / 供电模式 / 电量 / 信号强度，
    // 但字节顺序未确认。换电池改电量、拉远距离改信号，看哪个字节跟着变。
    [self log:@"%@ 预留 %02X %02X %02X %02X",
         prefix, record.reserved1, record.reserved2, record.reserved3, record.reserved4];
}

- (NSString *)reasonText:(VTMJMRBPHistoryEndReason)reason {
    switch (reason) {
        case VTMJMRBPHistoryEndReasonIdle:         return @"Idle（设备静默）";
        case VTMJMRBPHistoryEndReasonCancelled:    return @"Cancelled（主动中断）";
        case VTMJMRBPHistoryEndReasonDisconnected: return @"Disconnected（连接断开）";
        case VTMJMRBPHistoryEndReasonBusy:         return @"Busy（上一轮未结束）";
    }
    return @"未知";
}

- (NSString *)nowText {
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    formatter.dateFormat = @"yyyy-MM-dd HH:mm:ss";
    return [formatter stringFromDate:NSDate.date];
}

@end
