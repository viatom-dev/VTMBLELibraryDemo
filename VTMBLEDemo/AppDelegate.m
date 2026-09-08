//
//  AppDelegate.m
//  VTMBLEDemo
//

#import "AppDelegate.h"
#import "VTMDemoScanViewController.h"

#import <VTMBLELibrary/VTMBLELibrary.h>

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

    [self configureSDKLogging];

    VTMDemoScanViewController *scan = [[VTMDemoScanViewController alloc] init];
    UINavigationController *nav = [[UINavigationController alloc] initWithRootViewController:scan];

    self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.rootViewController = nav;
    [self.window makeKeyAndVisible];

    return YES;
}

#pragma mark - SDK 日志

/// SDK 的日志默认完全关闭（`VTMBLELogLevelOff`），不打开就什么都不记。
///
/// Demo 里打开到 Debug，这样每一帧收发报文都能在 Console.app 和「日志」页看到。
/// 生产环境请保持 `VTMBLELogLevelOff`：原始报文含患者生理数据。
/// 确实需要在生产环境观察流量形态时，用 `redactsPayload = YES`，
/// 只保留帧头 / 长度 / 指令这些路由字节，测量值以 `<N bytes redacted>` 代替。
- (void)configureSDKLogging {
    VTMBLELogger *logger = VTMBLELogger.sharedLogger;

    logger.level = VTMBLELogLevelDebug;

    // 镜像到 os_log，subsystem 为 com.viatom.VTMBLELibrary。
    // 在 Console.app 里按 category（Core / WBP02 / PM10 / JMRBP）过滤可以实时看流量。
    logger.consoleOutputEnabled = YES;

    // 内存环形缓冲。问题发生之后再回捞现场，是排查偶发问题的主要手段。
    logger.memoryBufferEnabled = YES;
    logger.memoryCapacity = 2000;

    logger.redactsPayload = NO;
}

@end
