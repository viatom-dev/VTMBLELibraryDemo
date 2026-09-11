//
//  VTMJMRBPEnum.h
//  VTMBLELibrary
//
//  Created by viatom on 2026/9/8.
//
//  调用方需要传入或读取的语义枚举。指令码不在此处，见私有头文件。
//
//  Semantic enums the caller passes in or reads back. Command codes are not
//  here; they live in a private header.
//

#ifndef VTMJMRBPEnum_h
#define VTMJMRBPEnum_h

#import <Foundation/Foundation.h>

/// 设备工作配置。传给 `-activateWithProfile:`。
///
/// The device's working profile. Passed to `-activateWithProfile:`.
///
/// @discussion 不传配置直接调 `-activate` 时，设备按自身默认配置工作。
///
/// Calling `-activate` without a profile lets the device use its own default.
///
/// @note **无论哪种配置，一次测量都只上报一条结果。** 三次平均值测量虽然连测三次、
/// 房颤测量虽然多做一项判定，`-receiveMeasurementResult:` 都只回调一次，
/// 相应地也只需要一次确认。
///
/// **Every measurement reports exactly one result, whatever the profile.**
/// Averaging takes three readings and rhythm detection adds a judgement, yet
/// `-receiveMeasurementResult:` still fires once and one acknowledgement is enough.
typedef enum : NSUInteger {
    VTMJMRBPProfileStandard = 0x01,  /// 常规 / Standard
    VTMJMRBPProfileRhythm   = 0x02,  /// 节律相关 / Rhythm oriented
    VTMJMRBPProfileCardiac  = 0x03,  /// 心脏相关 / Cardiac oriented
    VTMJMRBPProfileAveraged = 0x04,  /// 连续三次取均值 / Average of three consecutive measurements
} VTMJMRBPProfile;

/// 设备端的用户槽位。设备为每个槽位独立存储记录。
///
/// A user slot on the device. Records are stored per slot.
///
/// @discussion 协议文档只描述了槽位 1 与 2，槽位 3 / 4 与 `All` 取自厂商 SDK，
/// 具体设备是否支持需实测确认。设备不支持时通常不返回任何记录。
///
/// The protocol document only describes slots 1 and 2. Slots 3 / 4 and `All`
/// come from the vendor's own SDK; whether a given device supports them has to
/// be verified against real hardware. Unsupported slots usually return no
/// records at all.
typedef enum : NSUInteger {
    VTMJMRBPUser1   = 0x00,   /// 用户 1 / User 1
    VTMJMRBPUser2   = 0x01,   /// 用户 2 / User 2
    VTMJMRBPUser3   = 0x02,   /// 用户 3（待实测确认）/ User 3 (unverified)
    VTMJMRBPUser4   = 0x03,   /// 用户 4（待实测确认）/ User 4 (unverified)
    VTMJMRBPUserAll = 0x64,   /// 全部槽位（待实测确认）/ Every slot (unverified)
} VTMJMRBPUser;

/// 一轮历史记录同步的结束原因。
///
/// Why a round of history synchronization ended.
typedef enum : NSUInteger {
    /// 设备静默超过阈值，本轮结束。
    ///
    /// The device went silent past the timeout, so the round ended.
    ///
    /// @warning **不代表设备上的记录已全部取到。** 设备既不提供总条数也不提供结束标志，
    /// 因此无法区分「已传完」与「设备中途终止上传」。未取到的记录仍留在设备上，
    /// 下一次同步会重新出现。
    ///
    /// **This does not mean every record was received.** The device provides
    /// neither a total count nor an end marker, so "finished sending" cannot be
    /// told apart from "stopped sending halfway". Records that did not arrive
    /// stay on the device and show up again on the next sync.
    VTMJMRBPHistoryEndReasonIdle,
    /// 调用方调用了 `-cancelHistorySync`。
    /// The caller invoked `-cancelHistorySync`.
    VTMJMRBPHistoryEndReasonCancelled,
    /// 同步过程中连接断开。
    /// The connection dropped mid-sync.
    VTMJMRBPHistoryEndReasonDisconnected,
    /// 上一轮同步尚未结束，本次请求没有发出。records 为空数组。
    /// The previous round had not finished, so this request was never sent.
    /// `records` is an empty array.
    VTMJMRBPHistoryEndReasonBusy,
} VTMJMRBPHistoryEndReason;

#endif /* VTMJMRBPEnum_h */
