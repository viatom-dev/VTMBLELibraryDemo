//
//  VTMWBP02Object.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/10/27.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface VTMWBP02DataModel : NSObject

@property (nonatomic, assign) u_short sys;
@property (nonatomic, assign) u_short dia;
@property (nonatomic, assign) u_short rate;
// yyyy-MM-dd HH:mm:ss
@property (nonatomic, strong) NSString *date;
@property (nonatomic, assign) u_char errorCode;

@end


@interface VTMWBP02AutoModeState : NSObject

@property (nonatomic, assign) BOOL daylightMode;
@property (nonatomic, strong) NSString *daylightStartTime;
@property (nonatomic, strong) NSString *daylightEntTime;
@property (nonatomic, assign) NSInteger daylightInterval;

@property (nonatomic, assign) BOOL nightlyMode;
@property (nonatomic, strong) NSString *nightlyStartTime;
@property (nonatomic, strong) NSString *nightlyEntTime;
@property (nonatomic, assign) NSInteger nightlyInterval;

@end

@interface VTMWBP02BatteryModel : NSObject

@property (nonatomic, assign) NSInteger percent;

@property (nonatomic, getter=isCharging) BOOL charging;

@property (nonatomic, getter=isFull) BOOL full;

@end

NS_ASSUME_NONNULL_END
