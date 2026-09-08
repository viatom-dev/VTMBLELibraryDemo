//
//  VTMDemoScanViewController.m
//  VTMBLEDemo
//

#import "VTMDemoScanViewController.h"
#import "VTMDemoCentralManager.h"
#import "VTMDemoWBP02ViewController.h"
#import "VTMDemoPM10ViewController.h"
#import "VTMDemoJMRBPViewController.h"
#import "VTMDemoLogViewController.h"

static NSString *const kCellID = @"VTMDemoScanCell";

@interface VTMDemoScanViewController ()
    <UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate, VTMDemoCentralManagerDelegate>

@property (nonatomic, strong) VTMDemoCentralManager *centralManager;

@property (nonatomic, strong) UISegmentedControl *kindControl;
@property (nonatomic, strong) UITextField *filterField;
@property (nonatomic, strong) UILabel *stateLabel;
@property (nonatomic, strong) UITableView *tableView;
@property (nonatomic, strong) UIBarButtonItem *scanItem;

/// 正在等待连接结果的外设。连上之后据此跳转。
@property (nonatomic, strong, nullable) CBPeripheral *pendingPeripheral;

@end

@implementation VTMDemoScanViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.title = @"VTMBLELibrary Demo";
    self.view.backgroundColor = UIColor.systemBackgroundColor;

    [self buildSubviews];

    self.centralManager = [[VTMDemoCentralManager alloc] init];
    self.centralManager.delegate = self;

    [self updateStateLabel];
}

- (void)buildSubviews {
    self.navigationItem.rightBarButtonItems = @[
        [[UIBarButtonItem alloc] initWithTitle:@"日志"
                                        style:UIBarButtonItemStylePlain
                                       target:self
                                       action:@selector(showLog)],
    ];

    _scanItem = [[UIBarButtonItem alloc] initWithTitle:@"开始扫描"
                                                style:UIBarButtonItemStylePlain
                                               target:self
                                               action:@selector(toggleScan)];
    self.navigationItem.leftBarButtonItem = _scanItem;

    // 按型号总数生成，新增型号只要在 VTMDemoDeviceKind 里加一项即可。
    NSMutableArray <NSString *> *kindTitles = [NSMutableArray arrayWithCapacity:VTMDemoDeviceKindCount];
    for (NSInteger kind = 0; kind < VTMDemoDeviceKindCount; kind++) {
        [kindTitles addObject:VTMDemoDeviceKindName((VTMDemoDeviceKind)kind)];
    }
    _kindControl = [[UISegmentedControl alloc] initWithItems:kindTitles];
    _kindControl.selectedSegmentIndex = 0;
    _kindControl.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:_kindControl];

    _filterField = [[UITextField alloc] init];
    _filterField.placeholder = @"按名称过滤，留空显示全部";
    _filterField.borderStyle = UITextBorderStyleRoundedRect;
    _filterField.font = [UIFont systemFontOfSize:14];
    _filterField.autocorrectionType = UITextAutocorrectionTypeNo;
    _filterField.autocapitalizationType = UITextAutocapitalizationTypeNone;
    _filterField.clearButtonMode = UITextFieldViewModeWhileEditing;
    _filterField.returnKeyType = UIReturnKeyDone;
    _filterField.delegate = self;
    _filterField.translatesAutoresizingMaskIntoConstraints = NO;
    [_filterField addTarget:self
                     action:@selector(filterChanged)
           forControlEvents:UIControlEventEditingChanged];
    [self.view addSubview:_filterField];

    _stateLabel = [[UILabel alloc] init];
    _stateLabel.font = [UIFont systemFontOfSize:12];
    _stateLabel.textColor = UIColor.secondaryLabelColor;
    _stateLabel.numberOfLines = 0;
    _stateLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:_stateLabel];

    _tableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStylePlain];
    _tableView.dataSource = self;
    _tableView.delegate = self;
    _tableView.rowHeight = 56;
    _tableView.translatesAutoresizingMaskIntoConstraints = NO;
    [_tableView registerClass:UITableViewCell.class forCellReuseIdentifier:kCellID];
    [self.view addSubview:_tableView];

    UILayoutGuide *safe = self.view.safeAreaLayoutGuide;
    [NSLayoutConstraint activateConstraints:@[
        [_kindControl.topAnchor constraintEqualToAnchor:safe.topAnchor constant:12],
        [_kindControl.leadingAnchor constraintEqualToAnchor:safe.leadingAnchor constant:16],
        [_kindControl.trailingAnchor constraintEqualToAnchor:safe.trailingAnchor constant:-16],

        [_filterField.topAnchor constraintEqualToAnchor:_kindControl.bottomAnchor constant:10],
        [_filterField.leadingAnchor constraintEqualToAnchor:_kindControl.leadingAnchor],
        [_filterField.trailingAnchor constraintEqualToAnchor:_kindControl.trailingAnchor],

        [_stateLabel.topAnchor constraintEqualToAnchor:_filterField.bottomAnchor constant:8],
        [_stateLabel.leadingAnchor constraintEqualToAnchor:_kindControl.leadingAnchor],
        [_stateLabel.trailingAnchor constraintEqualToAnchor:_kindControl.trailingAnchor],

        [_tableView.topAnchor constraintEqualToAnchor:_stateLabel.bottomAnchor constant:8],
        [_tableView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [_tableView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [_tableView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],
    ]];
}

#pragma mark - 动作

- (void)toggleScan {
    if (self.centralManager.isScanning) {
        [self.centralManager stopScan];
    } else {
        [self.centralManager clearDiscoveries];
        [self.centralManager startScan];
    }
    [self updateStateLabel];
}

- (void)filterChanged {
    self.centralManager.nameFilter = self.filterField.text;
    [self.centralManager clearDiscoveries];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [textField resignFirstResponder];
    return YES;
}

- (void)showLog {
    [self.navigationController pushViewController:[[VTMDemoLogViewController alloc] init] animated:YES];
}

- (VTMDemoDeviceKind)selectedKind {
    NSInteger index = self.kindControl.selectedSegmentIndex;
    if (index < 0 || index >= VTMDemoDeviceKindCount) {
        return VTMDemoDeviceKindWBP02;
    }
    return (VTMDemoDeviceKind)index;
}

- (void)updateStateLabel {
    NSString *stateText;
    switch (self.centralManager.state) {
        case CBManagerStatePoweredOn:    stateText = @"蓝牙已就绪"; break;
        case CBManagerStatePoweredOff:   stateText = @"蓝牙已关闭，请在系统设置里打开"; break;
        case CBManagerStateUnauthorized: stateText = @"未授权蓝牙权限"; break;
        case CBManagerStateResetting:    stateText = @"蓝牙正在重置"; break;
        case CBManagerStateUnsupported:  stateText = @"本设备不支持 BLE（模拟器无法连接真实设备）"; break;
        case CBManagerStateUnknown:      stateText = @"蓝牙状态未知"; break;
    }

    self.stateLabel.text = [NSString stringWithFormat:@"%@ · %@ · 已发现 %lu 台",
                            stateText,
                            self.centralManager.isScanning ? @"扫描中" : @"未扫描",
                            (unsigned long)self.centralManager.discoveries.count];

    self.scanItem.title = self.centralManager.isScanning ? @"停止扫描" : @"开始扫描";
    self.scanItem.enabled = (self.centralManager.state == CBManagerStatePoweredOn);
}

#pragma mark - VTMDemoCentralManagerDelegate

- (void)centralManager:(VTMDemoCentralManager *)manager didUpdateState:(CBManagerState)state {
    [self updateStateLabel];
}

- (void)centralManagerDidUpdateDiscoveries:(VTMDemoCentralManager *)manager {
    [self.tableView reloadData];
    [self updateStateLabel];
}

- (void)centralManager:(VTMDemoCentralManager *)manager didConnectPeripheral:(CBPeripheral *)peripheral {
    if (![peripheral isEqual:self.pendingPeripheral]) {
        return;
    }
    self.pendingPeripheral = nil;

    // 连接成功。把外设交给对应的设备页，由它创建 session 并等部署完成。
    UIViewController *deviceVC = nil;
    switch (self.selectedKind) {
        case VTMDemoDeviceKindWBP02:
            deviceVC = [[VTMDemoWBP02ViewController alloc] initWithPeripheral:peripheral
                                                              centralManager:manager];
            break;
        case VTMDemoDeviceKindPM10:
            deviceVC = [[VTMDemoPM10ViewController alloc] initWithPeripheral:peripheral
                                                             centralManager:manager];
            break;
        case VTMDemoDeviceKindJMRBP:
            deviceVC = [[VTMDemoJMRBPViewController alloc] initWithPeripheral:peripheral
                                                              centralManager:manager];
            break;
    }
    [self.navigationController pushViewController:deviceVC animated:YES];
}

- (void)centralManager:(VTMDemoCentralManager *)manager
didFailToConnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error {
    self.pendingPeripheral = nil;
    [self.tableView reloadData];
    [self presentMessage:[NSString stringWithFormat:@"连接失败：%@",
                          error.localizedDescription ?: @"未知原因"]];
}

- (void)centralManager:(VTMDemoCentralManager *)manager
didDisconnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error {
    self.pendingPeripheral = nil;
    [self.tableView reloadData];

    // 断线后设备页上的 session 已经不可用了，退回扫描页。
    // 是否自动重连由 App 决定，SDK 不介入，Demo 选择不重连。
    if (self.navigationController.topViewController != self) {
        [self.navigationController popToViewController:self animated:YES];
    }
    if (error) {
        [self presentMessage:[NSString stringWithFormat:@"连接已断开：%@", error.localizedDescription]];
    }
}

#pragma mark - UITableView

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return (NSInteger)self.centralManager.discoveries.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:kCellID forIndexPath:indexPath];
    VTMDemoDiscovery *discovery = self.centralManager.discoveries[(NSUInteger)indexPath.row];

    cell.textLabel.text = discovery.displayName;
    cell.textLabel.font = [UIFont systemFontOfSize:15];

    if ([discovery.peripheral isEqual:self.pendingPeripheral]) {
        cell.accessoryView = ({
            UIActivityIndicatorView *spinner =
                [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleMedium];
            [spinner startAnimating];
            spinner;
        });
    } else {
        cell.accessoryView = nil;
        UILabel *rssi = [[UILabel alloc] init];
        rssi.text = [NSString stringWithFormat:@"%@ dBm", discovery.rssi];
        rssi.font = [UIFont monospacedDigitSystemFontOfSize:12 weight:UIFontWeightRegular];
        rssi.textColor = UIColor.secondaryLabelColor;
        [rssi sizeToFit];
        cell.accessoryView = rssi;
    }

    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];

    VTMDemoDiscovery *discovery = self.centralManager.discoveries[(NSUInteger)indexPath.row];
    self.pendingPeripheral = discovery.peripheral;
    [self.centralManager connectPeripheral:discovery.peripheral];

    [self.tableView reloadData];
    [self updateStateLabel];
}

#pragma mark - 私有

- (void)presentMessage:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil
                                                                  message:message
                                                           preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"好" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

@end
