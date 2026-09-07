// swift-tools-version:5.9
//
//  Package.swift
//  VTMBLELibrary
//
//  SPM 分发清单。指向仓库里已编译好的 XCFramework，不含源码。
//
//  在你的工程里通过 Xcode 的 File > Add Package Dependencies 添加本仓库地址即可。
//

import PackageDescription

let package = Package(
    name: "VTMBLELibrary",
    platforms: [
        .iOS(.v13)
    ],
    products: [
        .library(
            name: "VTMBLELibrary",
            targets: ["VTMBLELibrary"]
        )
    ],
    targets: [
        .binaryTarget(
            name: "VTMBLELibrary",
            path: "VTMBLELibrary/VTMBLELibrary.xcframework"
        )
    ]
)
