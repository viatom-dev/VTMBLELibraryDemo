//
//  VTMPM10Enum.h
//  VTMBLELibrary
//
//  Created by yangweichao on 2025/11/7.
//
//  调用方需要传入或读取的语义枚举。指令码不在此处，见私有头文件。
//
//  Semantic enums the caller passes in or reads back. Command codes are not
//  here; they live in a private header.
//

#ifndef VTMPM10Enum_h
#define VTMPM10Enum_h

#import <Foundation/Foundation.h>

/// 设备的界面语言。取值即协议里的语言码。
///
/// The device's UI language. The raw values are the protocol's language codes.
///
/// @discussion 读写都通过 `VTMPM10SettingsModel.language`。**不同固件支持的语言并不相同**，
/// 写入前请先用 `-requestParameters:` 读回 `supportLanguages` 并确认目标语言在列表内 ——
/// 不在列表内时库会退回英语。
///
/// Both read and written through `VTMPM10SettingsModel.language`. **Firmware
/// builds differ in which languages they support**, so read `supportLanguages`
/// back with `-requestParameters:` first and check that your target is in the
/// list; the library falls back to English when it is not.
typedef enum : NSUInteger {
    VTMPM10LanguageZH,  /// 简体中文 / Chinese (Simplified)
    VTMPM10LanguageEN,  /// 英语 / English
    VTMPM10LanguageIT,  /// 意大利语 / Italian
    VTMPM10LanguageRU,  /// 俄语 / Russian
    VTMPM10LanguageFR,  /// 法语 / French
    VTMPM10LanguageBG,  /// 保加利亚语 / Bulgarian
    VTMPM10LanguageKK,  /// 哈萨克语 / Kazakh
    VTMPM10LanguagePL,  /// 波兰语 / Polish
    VTMPM10LanguageUK,  /// 乌克兰语 / Ukrainian
    VTMPM10LanguageES,  /// 西班牙语 / Spanish
    VTMPM10LanguageSK,  /// 斯洛伐克语 / Slovak
    VTMPM10LanguagePT,  /// 葡萄牙语 / Portuguese
    VTMPM10LanguageTR,  /// 土耳其语 / Turkish
    VTMPM10LanguageDE,  /// 德语 / German
    VTMPM10LanguageJP,  /// 日语 / Japanese
    VTMPM10LanguageHI,  /// 印地语 / Hindi
    VTMPM10LanguageAR,  /// 阿拉伯语 / Arabic
    VTMPM10LanguageNL,  /// 荷兰语 / Dutch
} VTMPM10Language;

/// 拉取病例列表的方式。传给 `-requestCaseInfo:serialNumber:callback:`。
///
/// How to enumerate the case list. Passed to
/// `-requestCaseInfo:serialNumber:callback:`.
///
/// @discussion 调用方只应传 `All` / `Unupload` / `Uploaded` / `Target` 四个值。
/// `Next` 与 `Resent` 是库内部逐条推进与重发时用的，不要从外部传入。
///
/// Callers should only pass `All`, `Unupload`, `Uploaded` or `Target`. `Next`
/// and `Resent` are used internally to walk the list and to ask for a resend;
/// do not pass them in from outside.
typedef enum : NSUInteger {
    /// 取下一条。**库内部使用**，调用方不要传。
    /// Fetch the next entry. **Internal use**; do not pass it in.
    VTMPM10ReqCaseInfoNext = 1,
    /// 重发当前条。**库内部使用**，收到的报文长度不足时自动发出。
    /// Ask for a resend of the current entry. **Internal use**; sent
    /// automatically when a response arrives too short to parse.
    VTMPM10ReqCaseInfoResent,
    /// 全部病例。/ Every case.
    VTMPM10ReqCaseInfoAll,
    /// 仅未上传的病例。/ Only cases not yet marked as uploaded.
    VTMPM10ReqCaseInfoUnupload,
    /// 仅已上传的病例。
    ///
    /// Only cases already marked as uploaded.
    ///
    /// @warning 设备只给出「总条数」与「未上传条数」两个计数，没有「已上传条数」。
    /// 库用总条数作为收齐的判据，因此本取值可能永远凑不满而不回调。
    /// 建议改用 `All` 后在调用方按 `uploadState` 过滤。
    ///
    /// The device only reports a total count and an un-uploaded count; there is
    /// no uploaded count. The library uses the total as its completion
    /// criterion, so this case may never reach it and never fire the callback.
    /// Prefer `All` and filter by `uploadState` on your side.
    VTMPM10ReqCaseInfoUploaded,
    /// 从指定 `serialNumber` 开始。
    ///
    /// Start from the given `serialNumber`.
    ///
    /// @warning 这不是「只取这一条」。库会从该条开始继续逐条拉取，直到凑满设备给出的
    /// 总条数为止。只想要一条时请在回调里自行筛选。
    ///
    /// This is not "fetch only this one". The library keeps walking forward from
    /// that entry until it has collected the total count reported by the device.
    /// Pick the single entry you want out of the callback yourself.
    VTMPM10ReqCaseInfoTarget,
} VTMPM10ReqCaseInfo;

/// 下载病例波形数据时的分段方式。
///
/// Chunking mode used while downloading a case's waveform data.
///
/// @warning **纯库内部使用。** 没有任何公开 API 接收该类型 ——
/// `-requestCaseDataWithInfo:progressHandle:callback:` 自行在 `Zero` 与 `Next`
/// 之间推进。`Save` 与 `Delete` 是协议里定义但库尚未使用的取值。
/// 该枚举当前登记在公开头文件中属于历史遗留，会在下一个大版本移入私有头文件。
///
/// **Internal use only.** No public API takes this type:
/// `-requestCaseDataWithInfo:progressHandle:callback:` drives the transfer
/// between `Zero` and `Next` on its own. `Save` and `Delete` are defined by the
/// protocol but not used by the library yet. Exposing this enum in a public
/// header is a leftover; it will move to a private header in the next major
/// version.
typedef enum : NSUInteger {
    /// 从头开始下载。/ Start the transfer from the beginning.
    VTMPM10ReqCaseDataZero,
    /// 取下一组数据。/ Fetch the next group of packets.
    VTMPM10ReqCaseDataNext,
    /// 重发当前组。/ Ask for a resend of the current group.
    VTMPM10ReqCaseDataResent,
    /// 在设备上把该病例标记为已保存 / 已上传。库尚未使用。
    /// Mark the case as saved / uploaded on the device. Not used yet.
    VTMPM10ReqCaseDataSave = 126,
    /// 在设备上删除该病例。库尚未使用。
    /// Delete the case on the device. Not used yet.
    VTMPM10ReqCaseDataDelete = 127,
} VTMPM10ReqCaseData;

#endif /* VTMPM10Enum_h */
