//
//  VTMBLELogger.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/9/4.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// @brief 库内部日志的详细程度。/ Verbosity of the library's internal log.
typedef NS_ENUM(NSInteger, VTMBLELogLevel) {
    /// 什么都不记。默认值。/ Nothing is recorded. Default value.
    VTMBLELogLevelOff     = 0,
    /// 只记失败：写入失败、无外设、特征值缺失、服务发现失败、报文校验不通过。
    /// Failures only: write errors, no peripheral, missing characteristic,
    /// failed discovery, checksum mismatch.
    VTMBLELogLevelError   = 1,
    /// 已自恢复的异常：丢弃错位字节重新同步、请求重发、写入类型降级、收到未知指令码。
    /// Recoverable anomalies: bytes discarded while resyncing, resent requests,
    /// write type fallback, unknown response codes.
    VTMBLELogLevelWarning = 2,
    /// 会话生命周期：绑定外设、发现服务、匹配 rx / tx、部署完成、队列排空、开始下载。
    /// Session lifecycle: peripheral attached, services discovered, rx / tx
    /// matched, deployment finished, queue drained, download started.
    VTMBLELogLevelInfo    = 3,
    /// 每一帧收发的十六进制，以及指令分派（当前指令码 vs 期待应答码）。
    /// Every frame sent and received, in hex, plus command dispatch (current
    /// command code vs expected response code).
    VTMBLELogLevelDebug   = 4,
    /// 单帧内部细节：MTU 分片、缓冲残料、逐包下载进度、波形包。
    /// Detail inside a single frame: MTU chunking, buffer residue, per-packet
    /// download progress, waveform packets.
    VTMBLELogLevelVerbose = 5,
};

/// @brief 一条日志记录。/ One entry of the internal log.
@interface VTMBLELogRecord : NSObject

@property (nonatomic, readonly) VTMBLELogLevel level;
@property (nonatomic, readonly) NSDate *timestamp;
/// @brief 记录来源，取值为 `Core` / `WBP02` / `PM10` / `JMRBP`。
/// Origin of the entry: `Core`, `WBP02`, `PM10` or `JMRBP`.
@property (nonatomic, readonly, copy) NSString *category;
@property (nonatomic, readonly, copy) NSString *message;
/// @brief 所在函数，读导出的日志文件时用得上。
/// Enclosing function, useful when reading an exported log.
@property (nonatomic, readonly, copy) NSString *function;
@property (nonatomic, readonly) NSInteger line;

/// @brief 单行渲染结果，控制台输出与 `-exportText` 都用它。
///
/// Single-line rendering used by console output and by `-exportText`.
///
/// @discussion 格式：`HH:mm:ss.SSS [等级][分类] 消息  (函数:行号)`
///
/// Format: `HH:mm:ss.SSS [level][category] message  (function:line)`
@property (nonatomic, readonly, copy) NSString *formattedLine;

@end


/// @brief 库内部的日志器。/ The library's internal logger.
///
/// @discussion 日志默认完全关闭。排查问题时打开：
///
/// Logging is fully disabled by default. Turn it on while diagnosing a problem:
///
/// ```
/// VTMBLELogger.sharedLogger.level = VTMBLELogLevelDebug;
/// ```
///
/// 一条日志同时进三个去处，各自解决不同场景：
///
/// - **控制台**，通过 `os_log` 写入 subsystem `com.viatom.VTMBLELibrary`。
///   在 Console.app 里按 category 过滤即可实时观察通信流量。
/// - **内存环形缓冲**，上限为 `memoryCapacity` 条，进程存活期间一直保留。
///   问题已经发生之后再回捞现场，这是排查偶发问题最有效的方式。
/// - **`handler` 回调**，设置后宿主 App 可以把日志接进自己的体系。
///
/// 收到现场问题反馈时，调用 `-writeLogFileToDirectory:error:` 并把产出的文件附在反馈里。
///
/// Entries go to three places, each independently useful:
///
/// - **Console**, through `os_log` under the subsystem `com.viatom.VTMBLELibrary`.
///   Filter by category in Console.app to watch traffic live.
/// - **In-memory ring buffer**, capped at `memoryCapacity` entries and kept for
///   the lifetime of the process. Grabbing the tail after a failure has already
///   happened is the most effective way to chase intermittent problems.
/// - **`handler`**, if set, so the host app can forward entries into its own
///   log pipeline.
///
/// When a user reports a field problem, call `-writeLogFileToDirectory:error:`
/// and attach the resulting file to the report.
///
/// @warning 原始报文含患者生理数据。生产环境请保持 `level` 为 `VTMBLELogLevelOff`，
/// 或置 `redactsPayload` 为 `YES`，只记录报文形状而不记录报文内容。
/// 本类的所有成员可在任意线程调用。
///
/// Raw frames contain patient physiological data. Keep `level` at
/// `VTMBLELogLevelOff` in production builds, or set `redactsPayload` to `YES`
/// to record frame shape without frame content. All members of this class are
/// safe to call from any thread.
@interface VTMBLELogger : NSObject

@property (class, nonatomic, readonly) VTMBLELogger *sharedLogger NS_SWIFT_NAME(shared);

/// @brief 高于此等级的日志被丢弃。默认 `VTMBLELogLevelOff`。
/// Entries above this level are dropped. Defaults to `VTMBLELogLevelOff`.
@property (atomic, assign) VTMBLELogLevel level;

/// @brief 是否同时写入 `os_log`。默认 `YES`。
/// Mirrors entries to `os_log`. Defaults to `YES`.
@property (atomic, assign) BOOL consoleOutputEnabled;

/// @brief 是否保留在内存环形缓冲中。默认 `YES`。
/// Keeps entries in the in-memory ring buffer. Defaults to `YES`.
@property (atomic, assign) BOOL memoryBufferEnabled;

/// @brief 环形缓冲的条数上限。默认 `2000`。
///
/// Ring buffer size in entries. Defaults to `2000`.
///
/// @discussion 调小后在记录下一条日志时才生效。
///
/// Shrinking it takes effect when the next entry is recorded.
@property (atomic, assign) NSUInteger memoryCapacity;

/// @brief 只保留路由字节（帧头 / 长度 / 指令），其余报文内容以字节数代替。默认 `NO`。
///
/// Replaces frame payloads with their byte count, keeping only the routing
/// bytes (header / length / command). Defaults to `NO`.
///
/// @discussion 需要在生产环境观察通信流量形态、但不能记录测量值时用它。
///
/// Use this when you need traffic shape from a production build without
/// recording measurement values.
@property (atomic, assign) BOOL redactsPayload;

/// @brief 每条被接受的日志都会回调，运行在库内部的串行队列上。
///
/// Invoked for every accepted entry, on an internal serial queue.
///
/// @warning 不要在该 block 里回调 VTMBLELibrary。
///
/// Do not call back into VTMBLELibrary from this block.
@property (atomic, copy, nullable) void (^handler)(VTMBLELogRecord *record);

/// @brief 环形缓冲的当前内容，最早的在前。
/// Current contents of the ring buffer, oldest first.
- (NSArray<VTMBLELogRecord *> *)bufferedRecords;

/// @brief 把环形缓冲渲染成文本，一条一行。
/// Ring buffer rendered as text, one entry per line.
- (NSString *)exportText;

/// @brief 把 `-exportText` 的内容写成带时间戳的 `.log` 文件。
///
/// Writes `-exportText` to a timestamped `.log` file.
///
/// @param directory 目标目录，不存在时自动创建。/ Destination directory. Created if missing.
/// @param error 写入失败时被赋值。/ Populated when the write fails.
/// @return 写入成功返回文件 URL，失败返回 `nil`。/ URL of the written file, or `nil` on failure.
- (nullable NSURL *)writeLogFileToDirectory:(NSURL *)directory
                                      error:(NSError *_Nullable *_Nullable)error;

/// @brief 清空环形缓冲。/ Empties the ring buffer.
- (void)clearBuffer;

@end

NS_ASSUME_NONNULL_END
