//
//  VTMWBP02Header.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/18.
//
//  WBP02（动态血压计）模块的聚合头文件。
//  Aggregate header of the WBP02 module (ambulatory blood pressure monitor).
//

#ifndef VTMWBP02Header_h
#define VTMWBP02Header_h

#import <VTMBLELibrary/VTMWBP02BLESession.h>
#import <VTMBLELibrary/VTMWBP02Object.h>


// 字节序展开宏。LE / BE 表示源字节序，P2U16 / P2U32 表示读出 2 / 4 字节无符号整数。
// 用宏而不是手写移位，是为了让协议解析里的字节序一眼可辨。
//
// Byte-order macros. LE / BE is the byte order of the source bytes, P2U16 /
// P2U32 reads a 2- / 4-byte unsigned integer out of them. They exist so that
// the byte order at each protocol parsing site is obvious at a glance.
//
// 注意：这几个宏定义在公开头文件里，会进入集成方的宏命名空间。
// 与 VTMJMRBPHeader.h 中的 BE_P2U16 是完全相同的定义，两处都加了 #ifndef 卫哨，
// 因此伞头文件同时 import 两个模块时不会冲突。
//
// Note: these macros live in a public header and therefore land in the
// integrator's macro namespace. BE_P2U16 is defined identically in
// VTMJMRBPHeader.h; both sites are guarded with #ifndef, so importing both
// modules through the umbrella header does not conflict.

#ifndef LE_P2U16
#define LE_P2U16(p,u) do{u=0;u = (p)[0]|((p)[1]<<8);}while(0)
#endif
#ifndef LE_P2U32
#define LE_P2U32(p,u) do{u=0;u = (p)[0]|((p)[1]<<8)|((p)[2]<<16)|((p)[3]<<24);}while(0)
#endif
#ifndef BE_P2U16
#define BE_P2U16(p,u) do{u=0;u = ((p)[0]<<8)|((p)[1]);}while(0)
#endif
#ifndef BE_P2U32
#define BE_P2U32(p,u) do{u=0;u = ((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|((p)[3]);}while(0)
#endif


#endif /* VTMWBP02Header_h */
