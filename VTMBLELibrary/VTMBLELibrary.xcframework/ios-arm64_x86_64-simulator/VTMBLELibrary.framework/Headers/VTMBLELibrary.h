//
//  VTMBLELibrary.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/13.
//
//  伞头文件。集成方只需 `@import VTMBLELibrary;` 或
//  `#import <VTMBLELibrary/VTMBLELibrary.h>` 即可拿到全部对外 API。
//
//  Umbrella header. Integrators only need `@import VTMBLELibrary;` or
//  `#import <VTMBLELibrary/VTMBLELibrary.h>` to reach the whole public API.
//
//  ---------------------------------------------------------------------------
//  注释语言约定 / Comment language convention
//
//  公开头文件与对外文档一律「中文原文 + 英文翻译」双语：中文段落在前，
//  空行之后是对应的英文段落；单行的 @param / @return / 枚举项用
//  「中文。/ English.」的形式并排。库内部的实现文件与私有头文件只用简体中文。
//
//  Public headers and outward-facing docs are bilingual: the Chinese text comes
//  first, the matching English text follows after a blank comment line. Short
//  bodies (@param / @return / enum cases) put both on one line, separated by
//  ` / `. Implementation files and private headers are Simplified Chinese only.
//  ---------------------------------------------------------------------------
//

#import <Foundation/Foundation.h>

//! Project version number for VTMBLELibrary.
FOUNDATION_EXPORT double VTMBLELibraryVersionNumber;

//! Project version string for VTMBLELibrary.
FOUNDATION_EXPORT const unsigned char VTMBLELibraryVersionString[];

// 新增设备模块时在这里加一行该模块的 Header.h。
// 公开头文件清单需同步四处，见 README「目录结构」一节。
//
// When adding a device module, add one line for its Header.h here.
// The public header list is duplicated in four places; see "Layout" in README.

#import <VTMBLELibrary/VTMBLELogger.h>
#import <VTMBLELibrary/VTMBLECoreSession.h>
#import <VTMBLELibrary/VTMWBP02Header.h>
#import <VTMBLELibrary/VTMPM10Header.h>
#import <VTMBLELibrary/VTMJMRBPHeader.h>
