//
//  VTMWBP02Enum.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/19.
//

#ifndef VTMWBP02Enum_h
#define VTMWBP02Enum_h

typedef enum : u_char {
    VTMWBP02HeaderDefault = 0x5A,
} VTMWBP02Header;


typedef enum : u_char {
    VTMWBP02CMDHandshakes = 0x10,       /// 握手连接
    VTMWBP02CMDGetDeviceInfo = 0x11,    /// 获取设备信息
    VTMWBP02CMDSyncTime = 0x12,         /// 设置同步时间
    VTMWBP02CMDSetLoginUser = 0x13,     /// 设置登录用户
    VTMWBP02CMDGetHistory = 0x14,       /// 设置血压记录时间
    VTMWBP02CMDGetBattery = 0x15,       /// 主动获取电池电量
    VTMWBP02CMDMeasurementEnd = 0x17,   /// 测量结束
    
    VTMWBP02CMDSetDayAutoMode = 0x21,   /// 设置白天测量模式
    VTMWBP02CMDSetDayTime = 0x22,        /// 设置白天测量时间
    VTMWBP02CMDSetNightAutoMode = 0x23, /// 设置夜间测量模式
    VTMWBP02CMDSetNightTime =  0x24,    /// 设置夜间测量时间
    VTMWBP02CMDSetAutoModeEnd = 0x2A,   ///  编程完成
    
    
    VTMWBP02CMDReadBPData = 0x30,       /// 读取血压数据
    VTMWBP02CMDReeadBattery = 0x31,     /// 读取电量信息
    VTMWBP02CMDEndPressure = 0x36,      /// 结束打气， 手动停止
    VTMWBP02CMDReadBPDataEnd = 0x3A,    /// 读取血压数据结束
    
    VTMWBP02CMDRealTimeData = 0x50,      /// 实时袖带压
    
    VTMWBP02CMDGetAutoSetting = 0xF6,
    
} VTMWBP02CMD;

#endif /* VTMWBP02Enum_h */
