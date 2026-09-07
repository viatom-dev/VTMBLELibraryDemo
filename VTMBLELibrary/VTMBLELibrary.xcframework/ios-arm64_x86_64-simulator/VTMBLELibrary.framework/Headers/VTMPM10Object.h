//
//  VTMPM10Object.h
//  VTMBLELibrary
//
//  Created by yangweichao on 2025/11/10.
//

#import <Foundation/Foundation.h>
#import <VTMBLELibrary/VTMPM10Enum.h>

NS_ASSUME_NONNULL_BEGIN

@interface VTMPM10SettingsModel : NSObject

@property (nonatomic, assign) VTMPM10Language language;

@property (nonatomic, assign) BOOL filterSwi;

@property (nonatomic, assign) BOOL anti_aliasingSwi;

@property (nonatomic, assign) BOOL autoMeasureSwi;

// save duration 0-2  0: 10s  1: 15s  2: 30s
@property (nonatomic, assign) int duration;

@property (nonatomic, assign) BOOL heartSoundSwi;

@property (nonatomic, assign) BOOL analysisSwi;

@property (nonatomic, copy) NSArray <NSNumber *>*supportLanguages;

@end

@interface VTMPM10CaseInfoModel : NSObject

@property (nonatomic, assign) NSUInteger serialNumber;

@property (nonatomic, assign) BOOL uploadState;

@property (nonatomic, assign) NSUInteger lead;

/// yyyyMMddHHmmss
@property (nonatomic, copy) NSString *dateStr;

@property (nonatomic, assign) NSUInteger length;

@property (nonatomic, assign) NSUInteger heartRate;

@property (nonatomic, copy) NSArray <NSNumber *>*resultCodes;

@property (nonatomic, copy) NSData *contentData;

- (instancetype)initWithData:(NSData *)data;

/// support ZH and EN
- (NSString *)resultTextWithCode:(NSNumber *)code language:(VTMPM10Language)language;

@end

@interface VTMPM10CaseDataMdoel : NSObject

@property (nonatomic, strong) VTMPM10CaseInfoModel *infoModel;

@property (nonatomic, copy) NSArray *uVElements;

- (instancetype)initWitData:(NSData *)data;

@end


@interface VTMPM10DeviceModel : NSObject

@property (nonatomic, copy) NSString *software;

@property (nonatomic, copy) NSString *hardware;

@property (nonatomic, copy) NSString *snString;

// 标识
@property (nonatomic, copy) NSString *logoString;



@end

NS_ASSUME_NONNULL_END
