# Podfile
#
# 两种依赖模式，由环境变量 VTM_BLE_SDK_SOURCE 切换。
#
#   pod install
#       默认模式。用本仓库自带的 VTMBLELibrary.xcframework，
#       这就是第三方集成方看到的形态。
#
#   VTM_BLE_SDK_SOURCE=../path/to/vtmblelibrary pod install
#       源码模式，仅 Viatom 内部可用（需要私有源码仓）。
#       SDK 以源码参与编译，可以下断点、可以改，改完直接跑 Demo 验证。
#
# 两种模式共用同一份 Demo 代码。发版前请务必在默认模式下跑一遍 —— 公开头文件
# 漏登记这类问题，在源码模式下永远不会暴露。

platform :ios, '13.0'
use_frameworks! :linkage => :static

target 'VTMBLEDemo' do
  sdk_source = ENV['VTM_BLE_SDK_SOURCE']

  if sdk_source && !sdk_source.strip.empty?
    warn "==> VTMBLELibrary: 源码模式 (#{sdk_source})"
    pod 'VTMBLELibrary', :path => sdk_source
  else
    warn '==> VTMBLELibrary: 二进制模式 (VTMBLELibrary/VTMBLELibrary.xcframework)'
    pod 'VTMBLELibrary', :path => 'VTMBLELibrary'
  end
end

post_install do |installer|
  installer.pods_project.targets.each do |target|
    target.build_configurations.each do |config|
      # 统一部署目标，避免 Xcode 对低于当前 SDK 下限的 pod 报警告。
      config.build_settings['IPHONEOS_DEPLOYMENT_TARGET'] = '13.0'
    end
  end
end
