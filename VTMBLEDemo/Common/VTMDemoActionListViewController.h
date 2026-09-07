//
//  VTMDemoActionListViewController.h
//  VTMBLEDemo
//

#import <UIKit/UIKit.h>
#import "VTMDemoConsoleView.h"

NS_ASSUME_NONNULL_BEGIN

/// 列表里的一个可点条目，对应 SDK 的一次调用。
@interface VTMDemoAction : NSObject

@property (nonatomic, copy, readonly) NSString *title;
@property (nonatomic, copy, readonly, nullable) NSString *detail;
@property (nonatomic, copy, readonly) void (^handler)(void);

+ (instancetype)actionWithTitle:(NSString *)title
                         detail:(nullable NSString *)detail
                        handler:(void (^)(void))handler;

@end


@interface VTMDemoActionSection : NSObject

@property (nonatomic, copy, readonly) NSString *title;
@property (nonatomic, copy, readonly, nullable) NSString *footer;
@property (nonatomic, copy, readonly) NSArray <VTMDemoAction *> *actions;

+ (instancetype)sectionWithTitle:(NSString *)title
                          footer:(nullable NSString *)footer
                         actions:(NSArray <VTMDemoAction *> *)actions;

@end


/// 上半屏一张指令列表，下半屏一个输出面板。两个设备页共用这套骨架，
/// 子类只需要覆写 `-buildSections` 描述有哪些指令可点。
@interface VTMDemoActionListViewController : UIViewController

@property (nonatomic, strong, readonly) VTMDemoConsoleView *console;

/// 列表是否可点。设备页在 session 部署完成前置为 NO。
@property (nonatomic, assign) BOOL actionsEnabled;

/// 子类覆写，返回要展示的指令分组。
- (NSArray <VTMDemoActionSection *> *)buildSections;

/// 重新读取 `-buildSections` 并刷新列表。
- (void)reloadActions;

/// 往输出面板打一行。
- (void)log:(NSString *)format, ... NS_FORMAT_FUNCTION(1, 2);

@end

NS_ASSUME_NONNULL_END
