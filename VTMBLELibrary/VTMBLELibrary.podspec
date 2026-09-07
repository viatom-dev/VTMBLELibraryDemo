#
# VTMBLELibrary.podspec
#
# 二进制分发清单。对外只给编译好的 XCFramework，不含源码。
#
# 与私有源码仓根目录的同名 podspec 的区别：
#   - 那份用 `source_files` + `public_header_files`，是源码分发，仅内部使用
#   - 这份用 `vendored_frameworks`，是对外分发
# 两者的 `version` 必须一致。
#

Pod::Spec.new do |s|
  s.name             = 'VTMBLELibrary'
  s.version          = '1.0.0'
  s.summary          = 'iOS Bluetooth SDK for Viatom medical devices'

  s.description      = <<~DESC
    VTMBLELibrary handles the protocol layer on top of an already connected peripheral:
    command encoding, queueing, MTU based fragmentation, response de-framing and parsing.
    Scanning, connecting and reconnecting stay with your own CBCentralManager.
    Currently supports the WBP02 ambulatory blood pressure monitor and the PM10 handheld ECG.
  DESC

  # TODO(发布前确认): 换成实际的 GitHub 组织与仓库名。
  s.homepage         = 'https://github.com/viatomtech/VTMBLELibrary-Demo'
  s.author           = { 'Viatom' => 'ios@viatomtech.com' }
  s.license          = { :type => 'Proprietary', :file => '../LICENSE' }

  s.source           = { :http => "https://github.com/viatomtech/VTMBLELibrary-Demo/releases/download/v#{s.version}/VTMBLELibrary.xcframework.zip" }

  s.ios.deployment_target = '13.0'
  s.requires_arc     = true

  s.vendored_frameworks = 'VTMBLELibrary.xcframework'
  s.frameworks          = 'Foundation', 'CoreBluetooth'
end
