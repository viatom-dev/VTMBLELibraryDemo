//
//  VTMPM10BLESession.h
//  VTMBLELibrary
//
//  Created by yangweichao on 2025/11/7.
//
// PS: command one by one. Do not send multiple commands at the same time.

#import <VTMBLELibrary/VTMBLECoreSession.h>
#import <VTMBLELibrary/VTMPM10Enum.h>
@class VTMPM10DeviceModel,VTMPM10CaseInfoModel,VTMPM10SettingsModel;

NS_ASSUME_NONNULL_BEGIN

@interface VTMPM10BLESession : VTMBLECoreSession

+ (instancetype)session;

- (void)requestDeviceInfo:(void(^)(VTMPM10DeviceModel *deviceModel))callback;

/// @1 success @0  failed
- (void)syncDate:(NSDate * _Nullable)date callback:(void(^)(NSNumber *result))callback;

- (void)requestCaseInfo:(VTMPM10ReqCaseInfo)method serialNumber:(NSUInteger)sn callback:(void(^)(NSArray <VTMPM10CaseInfoModel *>*infos))callback;

- (void)requestCaseDataWithInfo:(VTMPM10CaseInfoModel *)model progressHandle:(void(^)(CGFloat progress))handle callback:(void(^)(NSData *data))callback;

- (void)requestParameters:(void(^)(VTMPM10SettingsModel *model))callback;

- (void)syncParameters:(VTMPM10SettingsModel *)model callback:(void(^)(NSNumber *result))callback;



@end

NS_ASSUME_NONNULL_END
