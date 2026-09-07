//
//  VTMBLECoreSession.h
//  VTMBLELibrary
//
//  Created by viatom on 2025/8/13.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@class VTMBLECoreSession;

NS_ASSUME_NONNULL_BEGIN

@protocol VTMBLECoreSessionDelegate <NSObject>

/// @brief Called when rx / tx characteristics have been discovered and the session is ready.
/// @discussion Do not send any command before this callback arrives.
- (void)sessionDeployCompletion:(VTMBLECoreSession *)session;

@end


/// @brief Base class of every device session.
/// @discussion The library does not manage `CBCentralManager`. Scan and connect with your own
/// central manager, then assign the connected peripheral to `peripheral` to start deployment.
/// Wait for `-sessionDeployCompletion:` before issuing commands.
@interface VTMBLECoreSession : NSObject

/// @brief Unavailable. Create a session with the device specific `+session` factory,
/// for example `+[VTMWBP02BLESession session]`.
/// @discussion A session must be bound to a device description object to know its
/// rx / tx characteristics, so a plain `-init` would produce an unusable instance.
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/// @brief Receives the deployment result of services and characteristics.
@property (nonatomic, weak, nullable) id <VTMBLECoreSessionDelegate> sessionDelegate;

/// @brief The connected peripheral.
/// @discussion Assigning a peripheral resets the session state and starts discovering
/// services and characteristics. `nil` until you attach one.
@property (nonatomic, strong, nullable) CBPeripheral *peripheral;

@end

NS_ASSUME_NONNULL_END
