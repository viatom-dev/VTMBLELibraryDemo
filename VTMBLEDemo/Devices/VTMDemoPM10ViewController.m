//
//  VTMDemoPM10ViewController.m
//  VTMBLEDemo
//

#import "VTMDemoPM10ViewController.h"

/// SDK 只给语言的**码值**，不提供文案 —— 界面语言该由调用方决定。
/// 这个映射表就是「文案归调用方」的示例。
static NSString *VTMDemoPM10LanguageCode(VTMPM10Language language) {
    static NSArray <NSString *> *codes = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        codes = @[@"ZH", @"EN", @"IT", @"RU", @"FR", @"BG", @"KK", @"PL", @"UK",
                  @"ES", @"SK", @"PT", @"TR", @"DE", @"JP", @"HI", @"AR", @"NL"];
    });
    if (language < codes.count) {
        return codes[language];
    }
    return [NSString stringWithFormat:@"未知(%lu)", (unsigned long)language];
}

/// 保存时长的码值含义见 `VTMPM10SettingsModel.duration` 的注释。
static NSString *VTMDemoPM10DurationText(int duration) {
    switch (duration) {
        case 0: return @"10s";
        case 1: return @"15s";
        case 2: return @"30s";
        default: return [NSString stringWithFormat:@"未知(%d)", duration];
    }
}


@interface VTMDemoPM10ViewController ()

@property (nonatomic, weak) VTMPM10BLESession *pm10Session;

/// 最近一次读到的病例列表。下载病例数据需要其中的 info 模型。
@property (nonatomic, copy, nullable) NSArray <VTMPM10CaseInfoModel *> *cases;

/// 最近一次读到的参数设置。写回参数前必须先读一次。
@property (nonatomic, strong, nullable) VTMPM10SettingsModel *settings;

@end

@implementation VTMDemoPM10ViewController

- (void)viewDidLoad {
    self.title = @"PM10";
    [super viewDidLoad];
}

- (VTMBLECoreSession *)makeSession {
    VTMPM10BLESession *session = [VTMPM10BLESession session];
    self.pm10Session = session;
    return session;
}

- (void)sessionDidDeploy {
    // PM10 不需要握手，部署完成即可发指令。
    [self log:@"PM10 无需握手，可直接发指令"];
}

#pragma mark - 指令列表

- (NSArray <VTMDemoActionSection *> *)buildSections {
    __weak typeof(self) weakSelf = self;

    VTMDemoActionSection *device = [VTMDemoActionSection
        sectionWithTitle:@"设备"
                  footer:@"指令必须逐条发送，等 callback 回来再点下一条。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"设备信息"
                               detail:@"-requestDeviceInfo:"
                              handler:^{ [weakSelf doRequestDeviceInfo]; }],
        [VTMDemoAction actionWithTitle:@"同步时间到当前时刻"
                               detail:@"-syncDate:callback:  传 nil 表示用 [NSDate date]"
                              handler:^{ [weakSelf doSyncDate]; }],
    ]];

    VTMDemoActionSection *params = [VTMDemoActionSection
        sectionWithTitle:@"参数设置"
                  footer:@"写回参数前必须先读一次：syncParameters: 需要一个完整的 model，"
                          "不能只填想改的字段。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"读取参数"
                               detail:@"-requestParameters:"
                              handler:^{ [weakSelf doRequestParameters]; }],
        [VTMDemoAction actionWithTitle:@"翻转滤波开关并写回"
                               detail:@"-syncParameters:callback:"
                              handler:^{ [weakSelf doToggleFilterAndSync]; }],
    ]];

    VTMDemoActionSection *caseList = [VTMDemoActionSection
        sectionWithTitle:@"病例列表"
                  footer:@"SDK 内部会按设备应答自动续发，攒齐全部条目后才回调一次。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"全部病例"
                               detail:@"-requestCaseInfo: VTMPM10ReqCaseInfoAll"
                              handler:^{ [weakSelf doRequestCaseInfo:VTMPM10ReqCaseInfoAll]; }],
        [VTMDemoAction actionWithTitle:@"未上传病例"
                               detail:@"-requestCaseInfo: VTMPM10ReqCaseInfoUnupload"
                              handler:^{ [weakSelf doRequestCaseInfo:VTMPM10ReqCaseInfoUnupload]; }],
        [VTMDemoAction actionWithTitle:@"已上传病例"
                               detail:@"-requestCaseInfo: VTMPM10ReqCaseInfoUploaded"
                              handler:^{ [weakSelf doRequestCaseInfo:VTMPM10ReqCaseInfoUploaded]; }],
    ]];

    VTMDemoActionSection *caseData = [VTMDemoActionSection
        sectionWithTitle:@"病例数据"
                  footer:@"需要先读一次病例列表。下载回调给的是原始 NSData，"
                          "波形解析由调用方负责。"
                 actions:@[
        [VTMDemoAction actionWithTitle:@"下载第一条病例的波形"
                               detail:@"-requestCaseDataWithInfo:progressHandle:callback:"
                              handler:^{ [weakSelf doDownloadFirstCase]; }],
        [VTMDemoAction actionWithTitle:@"打印病例结论码与文案"
                               detail:@"-resultTextWithCode:language:"
                              handler:^{ [weakSelf doPrintCaseResults]; }],
    ]];

    return @[device, params, caseList, caseData];
}

#pragma mark - 指令实现

- (void)doRequestDeviceInfo {
    [self log:@"→ 读取设备信息…"];

    __weak typeof(self) weakSelf = self;
    [self.pm10Session requestDeviceInfo:^(VTMPM10DeviceModel *deviceModel) {
        [weakSelf log:@"← SN=%@  软件=%@  硬件=%@  标识=%@",
             deviceModel.snString, deviceModel.software,
             deviceModel.hardware, deviceModel.logoString];
    }];
}

- (void)doSyncDate {
    [self log:@"→ 同步时间…"];

    __weak typeof(self) weakSelf = self;
    [self.pm10Session syncDate:nil callback:^(NSNumber *result) {
        // 约定：1 成功，0 失败。
        [weakSelf log:@"← 同步时间 %@", result.boolValue ? @"成功" : @"失败"];
    }];
}

- (void)doRequestParameters {
    [self log:@"→ 读取参数设置…"];

    __weak typeof(self) weakSelf = self;
    [self.pm10Session requestParameters:^(VTMPM10SettingsModel *model) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        self.settings = model;

        [self log:@"← 语言=%@  保存时长=%@", VTMDemoPM10LanguageCode(model.language),
             VTMDemoPM10DurationText(model.duration)];
        [self log:@"  滤波=%@  抗锯齿=%@  自动测量=%@  心音=%@  自动分析=%@",
             model.filterSwi ? @"开" : @"关",
             model.anti_aliasingSwi ? @"开" : @"关",
             model.autoMeasureSwi ? @"开" : @"关",
             model.heartSoundSwi ? @"开" : @"关",
             model.analysisSwi ? @"开" : @"关"];

        NSMutableArray <NSString *> *supported = [NSMutableArray arrayWithCapacity:model.supportLanguages.count];
        for (NSNumber *code in model.supportLanguages) {
            [supported addObject:VTMDemoPM10LanguageCode((VTMPM10Language)code.unsignedIntegerValue)];
        }
        [self log:@"  设备支持语言：%@", [supported componentsJoinedByString:@" "]];
    }];
}

- (void)doToggleFilterAndSync {
    if (!self.settings) {
        [self log:@"请先执行「读取参数」。写回需要一个完整的 model。"];
        return;
    }

    self.settings.filterSwi = !self.settings.filterSwi;
    [self log:@"→ 写回参数，滤波置为 %@…", self.settings.filterSwi ? @"开" : @"关"];

    __weak typeof(self) weakSelf = self;
    [self.pm10Session syncParameters:self.settings callback:^(NSNumber *result) {
        [weakSelf log:@"← 写回参数 %@", result.boolValue ? @"成功" : @"失败"];
    }];
}

- (void)doRequestCaseInfo:(VTMPM10ReqCaseInfo)method {
    [self log:@"→ 读取病例列表（method=%lu）…", (unsigned long)method];

    // serialNumber 只在 `VTMPM10ReqCaseInfoTarget` 方式下有意义（指定目标编号），
    // 其余方式传 0 即可。
    __weak typeof(self) weakSelf = self;
    [self.pm10Session requestCaseInfo:method serialNumber:0
                             callback:^(NSArray <VTMPM10CaseInfoModel *> *infos) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        self.cases = infos;
        [self log:@"← 共 %lu 条病例", (unsigned long)infos.count];

        for (VTMPM10CaseInfoModel *info in infos) {
            [self log:@"   #%lu  %@  导联%lu  %lu 字节  %lu bpm  已上传=%@",
                 (unsigned long)info.serialNumber,
                 info.dateStr,
                 (unsigned long)info.lead,
                 (unsigned long)info.length,
                 (unsigned long)info.heartRate,
                 info.uploadState ? @"是" : @"否"];
        }
    }];
}

- (void)doDownloadFirstCase {
    VTMPM10CaseInfoModel *target = self.cases.firstObject;
    if (!target) {
        [self log:@"请先执行一次「病例列表」。"];
        return;
    }

    [self log:@"→ 下载病例 #%lu（%lu 字节）…",
         (unsigned long)target.serialNumber, (unsigned long)target.length];

    __weak typeof(self) weakSelf = self;
    [self.pm10Session requestCaseDataWithInfo:target progressHandle:^(CGFloat progress) {
        // 进度回调很密集，这里按 10% 的粒度打印，避免把输出面板刷爆。
        static NSInteger lastBucket = -1;
        NSInteger bucket = (NSInteger)(progress * 10);
        if (bucket != lastBucket) {
            lastBucket = bucket;
            [weakSelf log:@"   下载进度 %.0f%%", progress * 100];
        }
    } callback:^(NSData *data) {
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) { return; }

        [self log:@"← 下载完成，%lu 字节", (unsigned long)data.length];

        // SDK 到这里就交差了：回调给的是**原始字节**，波形解析属于调用方的事。
        // `VTMPM10CaseDataMdoel` 是公开的可选解析入口（注意 initWitData: 的拼写
        // 是已发布 API 里的笔误，不能改）。生产代码里建议自己按协议文档解析，
        // 并对 data.length 做校验。
        VTMPM10CaseDataMdoel *parsed = [[VTMPM10CaseDataMdoel alloc] initWitData:data];
        [self log:@"  解析出 %lu 个采样点", (unsigned long)parsed.uVElements.count];
    }];
}

- (void)doPrintCaseResults {
    if (self.cases.count == 0) {
        [self log:@"请先执行一次「病例列表」。"];
        return;
    }

    for (VTMPM10CaseInfoModel *info in self.cases) {
        if (info.resultCodes.count == 0) {
            [self log:@"#%lu 无结论码", (unsigned long)info.serialNumber];
            continue;
        }

        NSMutableArray <NSString *> *texts = [NSMutableArray arrayWithCapacity:info.resultCodes.count];
        for (NSNumber *code in info.resultCodes) {
            // SDK 目前只内置了 ZH / EN 两种文案，而设备支持 18 种语言。
            // 需要覆盖更多语言时，请拿 resultCodes 的**原始码值**自己做本地化。
            NSString *text = [info resultTextWithCode:code language:VTMPM10LanguageZH];
            [texts addObject:[NSString stringWithFormat:@"%@=%@", code, text.length ? text : @"?"]];
        }
        [self log:@"#%lu 结论 %@", (unsigned long)info.serialNumber,
             [texts componentsJoinedByString:@"; "]];
    }
}

@end
