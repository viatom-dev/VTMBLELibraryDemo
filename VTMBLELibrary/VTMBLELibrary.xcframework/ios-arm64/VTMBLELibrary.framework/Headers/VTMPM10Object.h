//
//  VTMPM10Object.h
//  VTMBLELibrary
//
//  Created by yangweichao on 2025/11/10.
//

#import <Foundation/Foundation.h>
#import <VTMBLELibrary/VTMPM10Enum.h>

NS_ASSUME_NONNULL_BEGIN

/// @brief 设备的测量参数设置。
///
/// The device's measurement settings.
///
/// @discussion 读写共用该模型：先用 `-requestParameters:` 读回一份，改动需要改的字段，
/// 再整份传给 `-syncParameters:callback:`。写入是**整份覆盖**，不要自己 new 一个空模型
/// 去写，否则未设置的字段会被清零。
///
/// The same model is used in both directions: read one back with
/// `-requestParameters:`, change the fields you care about, then pass the whole
/// thing to `-syncParameters:callback:`. Writing **replaces every field**, so do
/// not build a fresh empty model — the fields you did not set would be zeroed.
@interface VTMPM10SettingsModel : NSObject

/// @brief 设备界面语言。
///
/// The device's UI language.
///
/// @warning 写入前请确认目标语言在 `supportLanguages` 内，不在列表内时库会退回英语。
///
/// Make sure the target language is in `supportLanguages` before writing; the
/// library falls back to English when it is not.
@property (nonatomic, assign) VTMPM10Language language;

/// @brief 是否开启滤波。/ Whether filtering is on.
@property (nonatomic, assign) BOOL filterSwi;

/// @brief 是否开启抗混叠。属性名带下划线，是已发布的公开 API，保留原拼写。
///
/// Whether anti-aliasing is on. The underscore in the property name is part of
/// the already published API and is kept as-is.
@property (nonatomic, assign) BOOL anti_aliasingSwi;

/// @brief 是否开启自动测量。/ Whether automatic measurement is on.
@property (nonatomic, assign) BOOL autoMeasureSwi;

/// @brief 单次测量时长的档位。`0` = 10 秒，`1` = 15 秒，`2` = 30 秒。
///
/// Duration preset of a single measurement. `0` = 10 s, `1` = 15 s, `2` = 30 s.
///
/// @warning 库不校验取值范围，超出 0~2 的值会原样写给设备。
///
/// The library does not range-check this; a value outside 0–2 is written to the
/// device as-is.
@property (nonatomic, assign) int duration;

/// @brief 是否开启心跳音。/ Whether the heartbeat sound is on.
@property (nonatomic, assign) BOOL heartSoundSwi;

/// @brief 是否开启设备端的自动分析。/ Whether on-device analysis is on.
@property (nonatomic, assign) BOOL analysisSwi;

/// @brief 该固件支持的语言列表，元素为 `VTMPM10Language` 的装箱值。
///
/// The languages this firmware supports, boxed `VTMPM10Language` values.
///
/// @discussion 由 `-requestParameters:` 从设备的语言位掩码解析得到，只读性质。
/// `-syncParameters:callback:` 会用它校验 `language`。
///
/// Parsed from the device's language bitmask by `-requestParameters:`; treat it
/// as read-only. `-syncParameters:callback:` validates `language` against it.
@property (nonatomic, copy) NSArray <NSNumber *>*supportLanguages;

@end


/// @brief 一条病例的信息（不含波形数据）。
///
/// The metadata of one case; the waveform itself is not included.
///
/// @discussion 由 `-requestCaseInfo:serialNumber:callback:` 成批给出。
/// 要拿波形数据，把本模型传给 `-requestCaseDataWithInfo:progressHandle:callback:`。
///
/// Delivered in batches by `-requestCaseInfo:serialNumber:callback:`. To get the
/// waveform, hand this model to
/// `-requestCaseDataWithInfo:progressHandle:callback:`.
@interface VTMPM10CaseInfoModel : NSObject

/// @brief 病例在设备上的序号，下载时用它定位。
/// The case's index on the device; used to address it when downloading.
@property (nonatomic, assign) NSUInteger serialNumber;

/// @brief 设备端是否已把该病例标记为已上传。
/// Whether the device has marked this case as uploaded.
@property (nonatomic, assign) BOOL uploadState;

/// @brief 导联标识，取自协议的 1 个 bit，取值 0 或 1。
///
/// Lead identifier, a single protocol bit, either 0 or 1.
///
/// @warning 库只透出原始位值，不做语义化映射。
///
/// The library exposes the raw bit without mapping it to a name.
@property (nonatomic, assign) NSUInteger lead;

/// @brief 测量时间，格式 `yyyyMMddHHmmss`（**无分隔符**）。
///
/// Measurement time, formatted as `yyyyMMddHHmmss` (**no separators**).
///
/// @discussion 由设备时钟给出。建议在读取病例前先调用 `-syncDate:callback:` 校时。
///
/// Comes from the device clock. Call `-syncDate:callback:` before reading cases.
@property (nonatomic, copy) NSString *dateStr;

/// @brief 波形数据的总字节数，用于计算下载进度。
/// Total byte count of the waveform data; used to compute download progress.
@property (nonatomic, assign) NSUInteger length;

/// @brief 平均心率，次 / 分。/ Average heart rate, beats per minute.
@property (nonatomic, assign) NSUInteger heartRate;

/// @brief 设备给出的诊断结论码，元素为 `NSNumber`。
///
/// Diagnostic result codes reported by the device, boxed as `NSNumber`.
///
/// @discussion 一条病例可能有多个结论。设备未检出任何异常时，数组为 `@[@0]`
/// 而不是空数组。码值对应的文案见 `-resultTextWithCode:language:`。
///
/// A case can carry several conclusions. When the device found nothing
/// abnormal the array is `@[@0]`, not an empty array. See
/// `-resultTextWithCode:language:` for the wording behind a code.
@property (nonatomic, copy) NSArray <NSNumber *>*resultCodes;

/// @brief 解析本模型所用的原始信息块，16 字节。
///
/// The raw 16-byte info block this model was parsed from.
///
/// @warning 这**不是**波形数据。波形数据要通过
/// `-requestCaseDataWithInfo:progressHandle:callback:` 单独下载。
/// 该字段主要供排查协议问题时比对原始字节。
///
/// This is **not** the waveform. The waveform has to be downloaded separately
/// through `-requestCaseDataWithInfo:progressHandle:callback:`. This field is
/// mainly here so that raw bytes can be compared while debugging the protocol.
@property (nonatomic, copy) NSData *contentData;

/// @brief 从 16 字节信息块反序列化。库内部使用，调用方一般不需要直接调用。
///
/// Deserializes from the 16-byte info block. Used internally; callers normally
/// do not need to call it.
///
/// @warning 不校验长度，传入短于 16 字节的数据会读到越界内存。
///
/// The length is not validated; passing fewer than 16 bytes reads out of
/// bounds.
- (instancetype)initWithData:(NSData *)data;

/// @brief 把诊断结论码转成可读文案。
///
/// Turns a diagnostic result code into readable text.
///
/// @discussion **只内置了简体中文与英文两种文案**，传入其余语言一律返回英文。
/// 设备本身支持 18 种语言，SDK 不打算覆盖 —— 需要其他语言时请在 App 侧
/// 用码值自行本地化。
///
/// **Only Simplified Chinese and English wordings are built in**; any other
/// language returns English. The device itself supports 18 languages and the
/// SDK does not try to cover them — localize the code values on your side when
/// you need another language.
///
/// @param code `resultCodes` 里的元素。/ An element of `resultCodes`.
/// @param language 只有 `VTMPM10LanguageZH` 会返回中文。
///                 Only `VTMPM10LanguageZH` yields Chinese.
/// @return 对应文案。固件返回超出已知范围的码值时，返回码值本身的字符串形式。
///         The matching text. When the firmware reports a code outside the
///         known range, the code itself is returned as a string.
- (NSString *)resultTextWithCode:(NSNumber *)code language:(VTMPM10Language)language;

@end


/// @brief 病例波形数据的解析器：把下载到的原始字节转成 µV 序列。
///
/// Decoder for a case's waveform: turns the downloaded raw bytes into a
/// sequence of µV samples.
///
/// @discussion `-requestCaseDataWithInfo:progressHandle:callback:` 的回调给出的是
/// **未解析的原始数据**，波形解析这一步在调用方手上：
///
/// `-requestCaseDataWithInfo:progressHandle:callback:` hands back **raw,
/// undecoded data**; turning it into a waveform is done on the caller's side:
///
/// ```
/// VTMPM10CaseDataMdoel *ecg = [[VTMPM10CaseDataMdoel alloc] initWitData:data];
/// NSArray<NSNumber *> *microVolts = ecg.uVElements;
/// ```
///
/// @warning 类名与初始化方法名都有拼写错误（`Mdoel` 应为 `Model`，`initWitData:`
/// 应为 `initWithData:`）。这是已发布的公开 API，为不破坏集成方编译而保留原拼写。
///
/// Both the class name and the initializer are misspelled (`Mdoel` should be
/// `Model`, `initWitData:` should be `initWithData:`). They are already
/// published API and are kept as-is so that integrator code keeps compiling.
@interface VTMPM10CaseDataMdoel : NSObject

/// @brief 该波形对应的病例信息。
///
/// The case metadata this waveform belongs to.
///
/// @warning **SDK 从不给它赋值，读出来恒为 `nil`。** 需要的话请在调用方自行填入
/// 下载时用的那个 `VTMPM10CaseInfoModel`。
///
/// **The SDK never assigns it; reading it always yields `nil`.** Fill it in
/// yourself with the `VTMPM10CaseInfoModel` you used for the download if you
/// need the association.
@property (nonatomic, strong) VTMPM10CaseInfoModel *infoModel;

/// @brief 解析出的心电采样点，单位 µV，元素为 `NSNumber`。
///
/// The decoded ECG samples in µV, boxed as `NSNumber`.
///
/// @discussion 已扣除 16384 的基线偏移，因此可能为负值。
///
/// The 16384 baseline offset is already subtracted, so values can be negative.
@property (nonatomic, copy) NSArray *uVElements;

/// @brief 解析下载得到的原始波形数据。
///
/// Decodes the raw waveform data returned by a download.
///
/// @discussion 数据按 15 字节一个单元、每单元 6 个采样点解析，
/// 起始的 16 字节是病例信息块，会被跳过；末段不足一个完整单元的字节直接丢弃。
///
/// The data is decoded in 15-byte units of 6 samples each. The leading 16 bytes
/// are the case info block and are skipped; a trailing partial unit is dropped.
///
/// @param data `-requestCaseDataWithInfo:progressHandle:callback:` 回调给出的数据，
///             原样传入即可。
///             The data handed to the
///             `-requestCaseDataWithInfo:progressHandle:callback:` callback,
///             passed through unchanged.
- (instancetype)initWitData:(NSData *)data;

@end


/// @brief 设备标识与版本信息。由 `-requestDeviceInfo:` 一次性给出。
///
/// Device identity and firmware versions, reported in one shot by
/// `-requestDeviceInfo:`.
@interface VTMPM10DeviceModel : NSObject

/// @brief 软件（固件）版本，形如 `1.2.3`。/ Software (firmware) version, e.g. `1.2.3`.
@property (nonatomic, copy) NSString *software;

/// @brief 硬件版本，形如 `1.2.3`。/ Hardware version, e.g. `1.2.3`.
@property (nonatomic, copy) NSString *hardware;

/// @brief 设备序列号。/ Device serial number.
@property (nonatomic, copy) NSString *snString;

/// @brief 产品标识串，8 个 ASCII 字符，用于区分产品型号。
/// Product logo string, 8 ASCII characters, identifying the product model.
@property (nonatomic, copy) NSString *logoString;

@end

NS_ASSUME_NONNULL_END
