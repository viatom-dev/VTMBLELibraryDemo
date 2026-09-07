//
//  VTMBLELogger.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/9/4.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// @brief Verbosity of the library's internal log.
typedef NS_ENUM(NSInteger, VTMBLELogLevel) {
    /// Nothing is recorded. Default value.
    VTMBLELogLevelOff     = 0,
    /// Failures only: write errors, malformed frames, command timeouts.
    VTMBLELogLevelError   = 1,
    /// Recoverable anomalies: discarded bytes, resent requests.
    VTMBLELogLevelWarning = 2,
    /// Session lifecycle: deployment, command start / finish.
    VTMBLELogLevelInfo    = 3,
    /// Every frame sent and received, in hex.
    VTMBLELogLevelDebug   = 4,
    /// Per-packet detail inside bulk transfers, buffer residue, progress.
    VTMBLELogLevelVerbose = 5,
};

/// @brief One entry of the internal log.
@interface VTMBLELogRecord : NSObject

@property (nonatomic, readonly) VTMBLELogLevel level;
@property (nonatomic, readonly) NSDate *timestamp;
/// @brief Origin of the entry, e.g. `Core`, `WBP02`, `PM10`.
@property (nonatomic, readonly, copy) NSString *category;
@property (nonatomic, readonly, copy) NSString *message;
/// @brief Enclosing function, useful when reading an exported log.
@property (nonatomic, readonly, copy) NSString *function;
@property (nonatomic, readonly) NSInteger line;

/// @brief Single-line rendering used by console output and by `-exportText`.
/// @discussion Format: `HH:mm:ss.SSS [level][category] message  (function:line)`
@property (nonatomic, readonly, copy) NSString *formattedLine;

@end


/// @brief The library's internal logger.
///
/// @discussion Logging is fully disabled by default. Turn it on while diagnosing a problem:
///
/// ```
/// VTMBLELogger.sharedLogger.level = VTMBLELogLevelDebug;
/// ```
///
/// Entries go to three places, each independently useful:
///
/// - **Console**, through `os_log` under the subsystem `com.viatom.VTMBLELibrary`.
///   Filter by category in Console.app to watch traffic live.
/// - **In-memory ring buffer**, capped at `memoryCapacity` entries. Survives until the
///   process exits, so you can grab the tail after a failure has already happened.
/// - **`handler`**, if set, so the host app can forward entries into its own log pipeline.
///
/// When a user reports a field problem, call `-writeLogFileToDirectory:error:` and attach
/// the resulting file to the report.
///
/// @warning Raw frames contain patient physiological data. Keep `level` at
/// `VTMBLELogLevelOff` in production builds, or set `redactsPayload` to `YES` to record
/// frame shape without frame content. All members of this class are safe to call
/// from any thread.
@interface VTMBLELogger : NSObject

@property (class, nonatomic, readonly) VTMBLELogger *sharedLogger NS_SWIFT_NAME(shared);

/// @brief Entries above this level are dropped. Defaults to `VTMBLELogLevelOff`.
@property (atomic, assign) VTMBLELogLevel level;

/// @brief Mirrors entries to `os_log`. Defaults to `YES`.
@property (atomic, assign) BOOL consoleOutputEnabled;

/// @brief Keeps entries in the in-memory ring buffer. Defaults to `YES`.
@property (atomic, assign) BOOL memoryBufferEnabled;

/// @brief Ring buffer size in entries. Defaults to `2000`.
/// @discussion Shrinking it takes effect when the next entry is recorded.
@property (atomic, assign) NSUInteger memoryCapacity;

/// @brief Replaces frame payloads with their byte count, keeping only the routing bytes
/// (header / length / command). Defaults to `NO`.
/// @discussion Use this when you need traffic shape from a production build without
/// recording measurement values.
@property (atomic, assign) BOOL redactsPayload;

/// @brief Invoked for every accepted entry, on an internal serial queue.
/// @warning Do not call back into VTMBLELibrary from this block.
@property (atomic, copy, nullable) void (^handler)(VTMBLELogRecord *record);

/// @brief Current contents of the ring buffer, oldest first.
- (NSArray<VTMBLELogRecord *> *)bufferedRecords;

/// @brief Ring buffer rendered as text, one entry per line.
- (NSString *)exportText;

/// @brief Writes `-exportText` to a timestamped `.log` file.
/// @param directory Destination directory. Created if missing.
/// @param error Populated when the write fails.
/// @return URL of the written file, or `nil` on failure.
- (nullable NSURL *)writeLogFileToDirectory:(NSURL *)directory
                                      error:(NSError *_Nullable *_Nullable)error;

/// @brief Empties the ring buffer.
- (void)clearBuffer;

@end

NS_ASSUME_NONNULL_END
