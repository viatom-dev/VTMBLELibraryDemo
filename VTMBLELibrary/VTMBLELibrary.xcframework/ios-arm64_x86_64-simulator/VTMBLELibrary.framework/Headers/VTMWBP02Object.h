//
//  VTMWBP02Object.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/10/27.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// @brief 一条血压测量记录。
///
/// One blood pressure measurement.
///
/// @discussion 设备存储的历史记录与一次实时测量的结果共用该模型：
/// 历史记录由 `-getHistory:callback:` 成批给出，实时结果由
/// `-receiveRealtimeData:` 的第二个参数给出。
///
/// The same model is used for stored history and for the result of a live
/// measurement: history arrives in batches from `-getHistory:callback:`, the
/// live result arrives as the second argument of `-receiveRealtimeData:`.
@interface VTMWBP02DataModel : NSObject

/// @brief 收缩压，mmHg。/ Systolic pressure, mmHg.
@property (nonatomic, assign) u_short sys;
/// @brief 舒张压，mmHg。/ Diastolic pressure, mmHg.
@property (nonatomic, assign) u_short dia;
/// @brief 心率，次 / 分。/ Pulse rate, beats per minute.
@property (nonatomic, assign) u_short rate;

/// @brief 测量时间，格式 `yyyy-MM-dd HH:mm:ss`。
///
/// Measurement time, formatted as `yyyy-MM-dd HH:mm:ss`.
///
/// @discussion 协议只到分钟，秒位恒为 `00`。时间由设备时钟给出，
/// `-handshakes:` 内部已顺带校时。
///
/// The protocol only carries minutes, so the seconds field is always `00`.
/// The value comes from the device clock, which `-handshakes:` sets as part of
/// its internal sequence.
@property (nonatomic, strong) NSString *date;

/// @brief 本次测量的错误码，`0` 表示测量有效。
///
/// Error code of this measurement. `0` means the measurement is valid.
///
/// @warning `-getHistory:callback:` 返回的记录里该值**恒为 `0`** ——
/// 库在拆帧时已经把 `errorCode != 0` 的无效记录丢掉了，不会交给调用方。
/// 只有 `-receiveRealtimeData:` 给出的实时结果才可能带非 0 值。
///
/// In records returned by `-getHistory:callback:` this is **always `0`**: the
/// library drops invalid records (`errorCode != 0`) while decoding and never
/// hands them to the caller. Only the live result from
/// `-receiveRealtimeData:` can carry a non-zero value.
@property (nonatomic, assign) u_char errorCode;

@end


/// @brief 设备当前的自动测量设置。/ The device's current automatic measurement settings.
///
/// @discussion 由 `-getAutoModeSetiingInfo:` 读回。白天与夜间是两套独立的配置。
///
/// Read back through `-getAutoModeSetiingInfo:`. Daytime and night-time are two
/// independent configurations.
@interface VTMWBP02AutoModeState : NSObject

/// @brief 白天自动测量是否开启。/ Whether daytime automatic measurement is on.
@property (nonatomic, assign) BOOL daylightMode;
/// @brief 白天时段的开始时间，格式 `HH:mm`。/ Start of the daytime window, formatted as `HH:mm`.
@property (nonatomic, strong) NSString *daylightStartTime;
/// @brief 白天时段的结束时间，格式 `HH:mm`。/ End of the daytime window, formatted as `HH:mm`.
///
/// @warning 属性名少了一个字母（`Ent` 应为 `End`）。这是已发布的公开 API，
/// 为不破坏集成方编译而保留原拼写。
///
/// The property name is misspelled (`Ent` should be `End`). It is already
/// published API and is kept as-is so that integrator code keeps compiling.
@property (nonatomic, strong) NSString *daylightEntTime;
/// @brief 白天时段内的测量间隔，单位分钟。/ Measurement interval inside the daytime window, in minutes.
@property (nonatomic, assign) NSInteger daylightInterval;

/// @brief 夜间自动测量是否开启。/ Whether night-time automatic measurement is on.
@property (nonatomic, assign) BOOL nightlyMode;
/// @brief 夜间时段的开始时间，格式 `HH:mm`。/ Start of the night-time window, formatted as `HH:mm`.
@property (nonatomic, strong) NSString *nightlyStartTime;
/// @brief 夜间时段的结束时间，格式 `HH:mm`。/ End of the night-time window, formatted as `HH:mm`.
///
/// @warning 同 `daylightEntTime`，属性名的 `Ent` 应为 `End`，为兼容而保留。
///
/// Same as `daylightEntTime`: `Ent` should be `End`, kept for compatibility.
@property (nonatomic, strong) NSString *nightlyEntTime;
/// @brief 夜间时段内的测量间隔，单位分钟。/ Measurement interval inside the night-time window, in minutes.
@property (nonatomic, assign) NSInteger nightlyInterval;

@end


/// @brief 电池状态。/ Battery state.
@interface VTMWBP02BatteryModel : NSObject

/// @brief 剩余电量百分比，0~100。/ Remaining charge in percent, 0 to 100.
@property (nonatomic, assign) NSInteger percent;

/// @brief 是否正在充电。/ Whether the device is charging.
@property (nonatomic, getter=isCharging) BOOL charging;

/// @brief 是否已充满。充满时 `charging` 同时为 `YES`。
///
/// Whether the battery is full. When it is, `charging` is `YES` as well.
@property (nonatomic, getter=isFull) BOOL full;

@end

NS_ASSUME_NONNULL_END
