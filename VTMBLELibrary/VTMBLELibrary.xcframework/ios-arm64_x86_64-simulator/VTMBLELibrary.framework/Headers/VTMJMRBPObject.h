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
///
/// One measurement. Live results and records stored on the device share this
/// model; use `isHistory` to tell them apart.
@interface VTMJMRBPRecordModel : NSObject

/// 收缩压，mmHg / Systolic pressure, mmHg
@property (nonatomic, assign) u_short sys;
/// 舒张压，mmHg / Diastolic pressure, mmHg
@property (nonatomic, assign) u_short dia;
/// 心率，次 / 分 / Pulse rate, beats per minute
@property (nonatomic, assign) u_short rate;

/// 是否检测到疑似房颤。设备上报非 0 即视为疑似。
///
/// Whether possible atrial fibrillation was detected. Any non-zero value
/// reported by the device counts as a positive.
@property (nonatomic, assign, getter=isAtrialFibrillation) BOOL atrialFibrillation;
/// 是否检测到不规则脉搏。设备上报非 0 即视为存在。
///
/// Whether an irregular pulse was detected. Any non-zero value reported by the
/// device counts as a positive.
@property (nonatomic, assign, getter=isIrregularPulse) BOOL irregularPulse;

/// 记录归属的设备端用户槽位。
///
/// The device-side user slot this record belongs to.
@property (nonatomic, assign) VTMJMRBPUser user;

/// 测量时间。格式 yyyy-MM-dd HH:mm:ss
///
/// Measurement time, formatted as yyyy-MM-dd HH:mm:ss.
///
/// @warning 由设备时钟给出，不是手机时间。设备断电后时钟可能归零，
/// 建议在读取记录前先调用 `-syncTime:`。
///
/// This comes from the device clock, not the phone. The clock can reset to zero
/// after the device loses power, so call `-syncTime:` before reading records.
@property (nonatomic, copy) NSString *date;

/// 数据模式的原始码值。`0xFF` 表示这是一条设备存储的历史记录，其余取值表示实时测量。
///
/// Raw data-mode code. `0xFF` marks a record stored on the device; any other
/// value marks a live measurement.
///
/// @discussion 非 `0xFF` 时的具体含义尚未实测确认，因此按原始码值透出。
///
/// What the non-`0xFF` values mean has not been verified against real hardware
/// yet, so the raw code is exposed as-is.
@property (nonatomic, assign) u_char dataMode;

/// 是否为设备存储的历史记录。由 `dataMode` 推导。
///
/// Whether this is a record stored on the device. Derived from `dataMode`.
@property (nonatomic, assign, getter=isHistory, readonly) BOOL history;

/// 协议标注为预留的字节，按设备上报顺序以原始码值透出，未做语义化映射。
///
/// Bytes the protocol marks as reserved, exposed as raw codes in the order the
/// device reports them, with no semantic mapping applied.
///
/// @discussion 这几个字节在协议文档中均标注为「预留」，但设备实际会填入内容。
/// 具体含义与字节顺序需真机实测确认，确认后会补充具名属性，届时本组属性保留不变。
///
/// The protocol document marks all of them as reserved, yet the device does put
/// content in them. Their meaning and ordering need to be confirmed against real
/// hardware; named properties will be added once they are, and this group will
/// be kept unchanged.
@property (nonatomic, assign) u_char reserved1;
@property (nonatomic, assign) u_char reserved2;
@property (nonatomic, assign) u_char reserved3;
@property (nonatomic, assign) u_char reserved4;

/// 从一整帧测量结果报文反序列化。
///
/// Deserializes from one complete measurement frame.
///
/// @param data 完整的一帧报文。长度不足时返回 `nil`。
///             One complete frame. Returns `nil` when it is too short.
- (nullable instancetype)initWithData:(NSData *)data;

@end


/// 测量过程中设备实时上报的袖带压力。
///
/// The cuff pressure the device reports live during a measurement.
@interface VTMJMRBPPressureModel : NSObject

/// 设备上报的原始码值。
///
/// The raw code reported by the device.
@property (nonatomic, assign) u_short rawValue;

/// 换算后的压力，单位 mmHg。
///
/// The converted pressure, in mmHg.
///
/// @warning 换算系数取自协议示例反推，真机实测确认前请同时参考 `rawValue`。
///
/// The conversion factor was derived from the example in the protocol document.
/// Until it is verified against real hardware, cross-check against `rawValue`.
@property (nonatomic, assign) double mmHg;

/// 从一整帧压力报文反序列化。
///
/// Deserializes from one complete pressure frame.
///
/// @param data 完整的一帧报文。长度不足时返回 `nil`。
///             One complete frame. Returns `nil` when it is too short.
- (nullable instancetype)initWithData:(NSData *)data;

@end

NS_ASSUME_NONNULL_END
