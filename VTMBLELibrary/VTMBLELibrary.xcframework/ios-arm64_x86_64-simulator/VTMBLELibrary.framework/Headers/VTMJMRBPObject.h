//
//  VTMJMRBPObject.h
//  VTMBLELibrary
//
//  Created by viatom on 2026/9/8.
//

#import <Foundation/Foundation.h>
#import <VTMBLELibrary/VTMJMRBPEnum.h>

NS_ASSUME_NONNULL_BEGIN

/// 一条测量记录。实时测量结果与设备存储的历史记录共用该模型，靠 `isHistory` 区分。
@interface VTMJMRBPRecordModel : NSObject

/// 收缩压，mmHg
@property (nonatomic, assign) u_short sys;
/// 舒张压，mmHg
@property (nonatomic, assign) u_short dia;
/// 心率，次 / 分
@property (nonatomic, assign) u_short rate;

/// 是否检测到疑似房颤。设备上报非 0 即视为疑似。
@property (nonatomic, assign, getter=isAtrialFibrillation) BOOL atrialFibrillation;
/// 是否检测到不规则脉搏。设备上报非 0 即视为存在。
@property (nonatomic, assign, getter=isIrregularPulse) BOOL irregularPulse;

/// 记录归属的设备端用户槽位。
@property (nonatomic, assign) VTMJMRBPUser user;

/// 测量时间。格式 yyyy-MM-dd HH:mm:ss
///
/// @warning 由设备时钟给出，不是手机时间。设备断电后时钟可能归零，
/// 建议在读取记录前先调用 `-syncTime:`。
@property (nonatomic, copy) NSString *date;

/// 数据模式的原始码值。`0xFF` 表示这是一条设备存储的历史记录，其余取值表示实时测量。
///
/// @discussion 非 `0xFF` 时的具体含义尚未实测确认，因此按原始码值透出。
@property (nonatomic, assign) u_char dataMode;

/// 是否为设备存储的历史记录。由 `dataMode` 推导。
@property (nonatomic, assign, getter=isHistory, readonly) BOOL history;

/// 协议标注为预留的字节，按设备上报顺序以原始码值透出，未做语义化映射。
///
/// @discussion 这几个字节在协议文档中均标注为「预留」，但设备实际会填入内容。
/// 具体含义与字节顺序需真机实测确认，确认后会补充具名属性，届时本组属性保留不变。
@property (nonatomic, assign) u_char reserved1;
@property (nonatomic, assign) u_char reserved2;
@property (nonatomic, assign) u_char reserved3;
@property (nonatomic, assign) u_char reserved4;

/// 从一整帧测量结果报文反序列化。
/// @param data 完整的一帧报文。长度不足时返回 `nil`。
- (nullable instancetype)initWithData:(NSData *)data;

@end


/// 测量过程中设备实时上报的袖带压力。
@interface VTMJMRBPPressureModel : NSObject

/// 设备上报的原始码值。
@property (nonatomic, assign) u_short rawValue;

/// 换算后的压力，单位 mmHg。
///
/// @warning 换算系数取自协议示例反推，真机实测确认前请同时参考 `rawValue`。
@property (nonatomic, assign) double mmHg;

/// 从一整帧压力报文反序列化。
/// @param data 完整的一帧报文。长度不足时返回 `nil`。
- (nullable instancetype)initWithData:(NSData *)data;

@end

NS_ASSUME_NONNULL_END
