//
//  VTMWBP02BLESession.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/13.
//

#import <VTMBLELibrary/VTMBLECoreSession.h>

@class VTMWBP02DataModel, VTMWBP02AutoModeState, VTMWBP02BatteryModel;

NS_ASSUME_NONNULL_BEGIN

@interface VTMWBP02BLESession : VTMBLECoreSession

+ (instancetype)session;


- (void)handshakes:(void(^)(NSString *deviceSN))callback;

- (void)getHistory:(NSDate *)date callback:(void(^)(NSArray *results))callback;

- (void)getBatteyInfo:(void(^)(VTMWBP02BatteryModel *model))callback;

- (void)setDaylightAutoMode:(BOOL)autoMode callback:(void(^)(NSNumber *autoMode))callback;

- (void)setDaylightAutoStartTime:(NSString *)startTime endTime:(NSString *)endTime timeInterval:(NSInteger)timeInterval callback:(void(^)(NSNumber *success))callback;

- (void)setNightlyAutoMode:(BOOL)autoMode callback:(void(^)(NSNumber *autoMode))callback;

- (void)setNightlyAutoStartTime:(NSString *)startTime endTime:(NSString *)endTime timeInterval:(NSInteger)timeInterval callback:(void(^)(NSNumber *success))callback ;

- (void)getAutoModeSetiingInfo:(void(^)(VTMWBP02AutoModeState *autoState))callback;

- (void)receiveRealtimeData:(void(^)(NSNumber *pressure, VTMWBP02DataModel *result))realtimeData;

- (void)monitorPressureEnd:(void(^)(BOOL end))callback;

@end

NS_ASSUME_NONNULL_END
