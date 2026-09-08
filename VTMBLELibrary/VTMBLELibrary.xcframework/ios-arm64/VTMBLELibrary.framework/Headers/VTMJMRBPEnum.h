//
//  VTMJMRBPEnum.h
//  VTMBLELibrary
//
//  Created by viatom on 2026/9/8.
//
//  调用方需要传入或读取的语义枚举。指令码不在此处，见私有头文件。
//

#ifndef VTMJMRBPEnum_h
#define VTMJMRBPEnum_h

#import <Foundation/Foundation.h>

/// 设备工作配置。传给 `-activateWithProfile:`。
///
/// @discussion 不传配置直接调 `-activate` 时，设备按自身默认配置工作。
typedef enum : NSUInteger {
    VTMJMRBPProfileStandard = 0x01,  /// 常规
    VTMJMRBPProfileRhythm   = 0x02,  /// 节律相关
    VTMJMRBPProfileCardiac  = 0x03,  /// 心脏相关
    VTMJMRBPProfileAveraged = 0x04,  /// 连续三次取均值
} VTMJMRBPProfile;

/// 设备端的用户槽位。设备为每个槽位独立存储记录。
///
/// @discussion 协议文档只描述了槽位 1 与 2，槽位 3 / 4 与 `All` 取自厂商 SDK，
/// 具体设备是否支持需实测确认。设备不支持时通常不返回任何记录。
typedef enum : NSUInteger {
    VTMJMRBPUser1   = 0x00,
    VTMJMRBPUser2   = 0x01,
    VTMJMRBPUser3   = 0x02,
    VTMJMRBPUser4   = 0x03,
    VTMJMRBPUserAll = 0x64,
} VTMJMRBPUser;

/// 一轮历史记录同步的结束原因。
typedef enum : NSUInteger {
    /// 设备静默超过阈值，本轮结束。
    ///
    /// @warning **不代表设备上的记录已全部取到。** 设备既不提供总条数也不提供结束标志，
    /// 因此无法区分「已传完」与「设备中途终止上传」。未取到的记录仍留在设备上，
    /// 下一次同步会重新出现。
    VTMJMRBPHistoryEndReasonIdle,
    /// 调用方调用了 `-cancelHistorySync`。
    VTMJMRBPHistoryEndReasonCancelled,
    /// 同步过程中连接断开。
    VTMJMRBPHistoryEndReasonDisconnected,
    /// 上一轮同步尚未结束，本次请求没有发出。records 为空数组。
    VTMJMRBPHistoryEndReasonBusy,
} VTMJMRBPHistoryEndReason;

#endif /* VTMJMRBPEnum_h */
