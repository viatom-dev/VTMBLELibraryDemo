//
//  VTMDemoConsoleView.h
//  VTMBLEDemo
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

/// 屏幕下半部分的输出面板。把 SDK 的 callback 结果直接打在界面上，
/// 免得读 Demo 的人必须开着 Xcode 才能看到结果。
///
/// The output panel in the bottom half of the screen. It prints SDK callback
/// results straight onto the UI, so that reading the demo does not require having
/// Xcode attached.
@interface VTMDemoConsoleView : UIView

/// 追加一行，自动加时间戳并滚到底部。可在任意线程调用。
///
/// Appends a line, timestamping it and scrolling to the bottom. Safe to call from
/// any thread.
- (void)appendLine:(NSString *)line;

- (void)appendFormat:(NSString *)format, ... NS_FORMAT_FUNCTION(1, 2);

- (void)clear;

@end

NS_ASSUME_NONNULL_END
