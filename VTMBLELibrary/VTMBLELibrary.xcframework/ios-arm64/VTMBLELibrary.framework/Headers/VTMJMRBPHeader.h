//
//  VTMJMRBPHeader.h
//  VTMBLELibrary
//
//  Created by viatom on 2026/9/8.
//
//  JMRBP（蓝牙血压计）模块的聚合头文件。
//  Aggregate header of the JMRBP module (Bluetooth blood pressure monitor).
//

#ifndef VTMJMRBPHeader_h
#define VTMJMRBPHeader_h

#import <VTMBLELibrary/VTMJMRBPBLESession.h>
#import <VTMBLELibrary/VTMJMRBPEnum.h>
#import <VTMBLELibrary/VTMJMRBPObject.h>

// 字节序宏。与 VTMWBP02Header.h 中的定义完全一致，加卫哨避免重复定义
// —— 伞头文件会同时 import 两个设备的 Header.h。
//
// Byte-order macro, defined identically in VTMWBP02Header.h. The guard avoids a
// duplicate definition, since the umbrella header imports both device headers.
#ifndef BE_P2U16
#define BE_P2U16(p,u) do{u=0;u = ((p)[0]<<8)|((p)[1]);}while(0)
#endif

#endif /* VTMJMRBPHeader_h */
