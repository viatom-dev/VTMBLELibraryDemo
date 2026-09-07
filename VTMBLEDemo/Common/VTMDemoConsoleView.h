//
//  VTMDemoConsoleView.h
//  VTMBLEDemo
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

/// 屏幕下半部分的输出面板。把 SDK 的 callback 结果直接打在界面上，
/// 免得读 Demo 的人必须开着 Xcode 才能看到结果。
@interface VTMDemoConsoleView : UIView

/// 追加一行，自动加时间戳并滚到底部。可在任意线程调用。
- (void)appendLine:(NSString *)line;

- (void)appendFormat:(NSString *)format, ... NS_FORMAT_FUNCTION(1, 2);

- (void)clear;

@end

NS_ASSUME_NONNULL_END
