//
//  VTMDemoLogViewController.m
//  VTMBLEDemo
//

#import "VTMDemoLogViewController.h"
#import "VTMDemoConsoleView.h"

#import <VTMBLELibrary/VTMBLELibrary.h>

@interface VTMDemoLogViewController ()

@property (nonatomic, strong) UISegmentedControl *levelControl;
@property (nonatomic, strong) UISwitch *redactSwitch;
@property (nonatomic, strong) VTMDemoConsoleView *console;

@end

@implementation VTMDemoLogViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.title = @"SDK 日志";
    self.view.backgroundColor = UIColor.systemBackgroundColor;

    [self buildSubviews];
    [self loadBufferedRecords];
    [self installLiveHandler];
}

- (void)dealloc {
    // handler 是全局单例上的属性，页面走了必须摘掉，否则会一直持有已销毁的 self
    // 所捕获的对象。
    VTMBLELogger.sharedLogger.handler = nil;
}

- (void)buildSubviews {
    self.navigationItem.rightBarButtonItem =
        [[UIBarButtonItem alloc] initWithTitle:@"导出"
                                        style:UIBarButtonItemStylePlain
                                       target:self
                                       action:@selector(exportLogFile)];

    UILabel *levelLabel = [[UILabel alloc] init];
    levelLabel.text = @"等级";
    levelLabel.font = [UIFont systemFontOfSize:13 weight:UIFontWeightMedium];
    levelLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:levelLabel];

    _levelControl = [[UISegmentedControl alloc] initWithItems:@[@"Off", @"Err", @"Warn", @"Info", @"Debug", @"Verb"]];
    _levelControl.selectedSegmentIndex = VTMBLELogger.sharedLogger.level;
    _levelControl.translatesAutoresizingMaskIntoConstraints = NO;
    [_levelControl addTarget:self action:@selector(levelChanged) forControlEvents:UIControlEventValueChanged];
    [self.view addSubview:_levelControl];

    UILabel *redactLabel = [[UILabel alloc] init];
    redactLabel.text = @"报文脱敏（只留路由字节）";
    redactLabel.font = [UIFont systemFontOfSize:13];
    redactLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:redactLabel];

    _redactSwitch = [[UISwitch alloc] init];
    _redactSwitch.on = VTMBLELogger.sharedLogger.redactsPayload;
    _redactSwitch.translatesAutoresizingMaskIntoConstraints = NO;
    [_redactSwitch addTarget:self action:@selector(redactChanged) forControlEvents:UIControlEventValueChanged];
    [self.view addSubview:_redactSwitch];

    UILabel *hint = [[UILabel alloc] init];
    hint.text = @"原始报文含患者生理数据。生产环境请保持 Off，"
                 "或打开脱敏后再记录。Debug / Verbose 走 os_log 的 debug 类型，"
                 "系统默认不落盘。";
    hint.font = [UIFont systemFontOfSize:11];
    hint.textColor = UIColor.secondaryLabelColor;
    hint.numberOfLines = 0;
    hint.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:hint];

    _console = [[VTMDemoConsoleView alloc] initWithFrame:CGRectZero];
    _console.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:_console];

    UILayoutGuide *safe = self.view.safeAreaLayoutGuide;
    [NSLayoutConstraint activateConstraints:@[
        [levelLabel.topAnchor constraintEqualToAnchor:safe.topAnchor constant:12],
        [levelLabel.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:16],

        [_levelControl.topAnchor constraintEqualToAnchor:levelLabel.bottomAnchor constant:6],
        [_levelControl.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:16],
        [_levelControl.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-16],

        [redactLabel.topAnchor constraintEqualToAnchor:_levelControl.bottomAnchor constant:16],
        [redactLabel.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:16],
        [_redactSwitch.centerYAnchor constraintEqualToAnchor:redactLabel.centerYAnchor],
        [_redactSwitch.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-16],
        [_redactSwitch.leadingAnchor constraintGreaterThanOrEqualToAnchor:redactLabel.trailingAnchor constant:8],

        [hint.topAnchor constraintEqualToAnchor:redactLabel.bottomAnchor constant:12],
        [hint.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:16],
        [hint.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-16],

        [_console.topAnchor constraintEqualToAnchor:hint.bottomAnchor constant:12],
        [_console.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [_console.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [_console.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],
    ]];
}

#pragma mark - 日志

/// 进页面时先把环形缓冲里已有的日志倒出来。
/// 这是排查偶发问题的主要手段：问题发生之后再回捞现场。
- (void)loadBufferedRecords {
    for (VTMBLELogRecord *record in VTMBLELogger.sharedLogger.bufferedRecords) {
        [self.console appendLine:record.formattedLine];
    }
}

- (void)installLiveHandler {
    __weak typeof(self) weakSelf = self;
    VTMBLELogger.sharedLogger.handler = ^(VTMBLELogRecord *record) {
        // handler 在 logger 的内部串行队列上被调用。
        // 两条纪律：
        //   1. 不要在这里回调 VTMBLELibrary 的任何方法
        //   2. 要动 UI 就切主线程
        // 这里只取出已经格式化好的字符串，其余交给 console（它内部会切主线程）。
        NSString *line = record.formattedLine;
        [weakSelf.console appendLine:line];
    };
}

- (void)levelChanged {
    VTMBLELogger.sharedLogger.level = (VTMBLELogLevel)self.levelControl.selectedSegmentIndex;
    [self.console appendFormat:@"-- 日志等级已设为 %ld --", (long)VTMBLELogger.sharedLogger.level];
}

- (void)redactChanged {
    VTMBLELogger.sharedLogger.redactsPayload = self.redactSwitch.isOn;
    [self.console appendFormat:@"-- 报文脱敏 %@ --", self.redactSwitch.isOn ? @"已开启" : @"已关闭"];
}

/// 现场问题的标准取证方式：导出成文件，让用户把文件发回来。
- (void)exportLogFile {
    NSURL *directory = [NSFileManager.defaultManager URLsForDirectory:NSCachesDirectory
                                                           inDomains:NSUserDomainMask].firstObject;
    if (!directory) {
        [self presentMessage:@"拿不到 Caches 目录"];
        return;
    }

    NSError *error = nil;
    NSURL *logFile = [VTMBLELogger.sharedLogger writeLogFileToDirectory:directory error:&error];
    if (!logFile) {
        [self presentMessage:[NSString stringWithFormat:@"导出失败：%@", error.localizedDescription]];
        return;
    }

    UIActivityViewController *share =
        [[UIActivityViewController alloc] initWithActivityItems:@[logFile] applicationActivities:nil];
    share.popoverPresentationController.barButtonItem = self.navigationItem.rightBarButtonItem;
    [self presentViewController:share animated:YES completion:nil];
}

- (void)presentMessage:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil
                                                                  message:message
                                                           preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"好" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

@end
