//
//  VTMDemoActionListViewController.m
//  VTMBLEDemo
//

#import "VTMDemoActionListViewController.h"

#pragma mark -

@interface VTMDemoAction ()
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy, nullable) NSString *detail;
@property (nonatomic, copy) void (^handler)(void);
@end

@implementation VTMDemoAction

+ (instancetype)actionWithTitle:(NSString *)title
                         detail:(NSString *)detail
                        handler:(void (^)(void))handler {
    VTMDemoAction *action = [[self alloc] init];
    action.title = title;
    action.detail = detail;
    action.handler = handler;
    return action;
}

@end


#pragma mark -

@interface VTMDemoActionSection ()
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy, nullable) NSString *footer;
@property (nonatomic, copy) NSArray <VTMDemoAction *> *actions;
@end

@implementation VTMDemoActionSection

+ (instancetype)sectionWithTitle:(NSString *)title
                          footer:(NSString *)footer
                         actions:(NSArray <VTMDemoAction *> *)actions {
    VTMDemoActionSection *section = [[self alloc] init];
    section.title = title;
    section.footer = footer;
    section.actions = actions;
    return section;
}

@end


#pragma mark -

/// `registerClass:` 会用 `UITableViewCellStyleDefault` 建 cell，那种样式下
/// `detailTextLabel` 是 nil，副标题写不进去。这里固定成 Subtitle 样式。
@interface VTMDemoActionCell : UITableViewCell
@end

@implementation VTMDemoActionCell

- (instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    self = [super initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:reuseIdentifier];
    if (self) {
        self.detailTextLabel.numberOfLines = 0;
        self.detailTextLabel.font = [UIFont systemFontOfSize:11];
        self.detailTextLabel.textColor = UIColor.secondaryLabelColor;
    }
    return self;
}

@end


static NSString *const kCellID = @"VTMDemoActionCell";

@interface VTMDemoActionListViewController () <UITableViewDataSource, UITableViewDelegate>

@property (nonatomic, strong) UITableView *tableView;
@property (nonatomic, strong) VTMDemoConsoleView *console;
@property (nonatomic, copy) NSArray <VTMDemoActionSection *> *sections;

@end

@implementation VTMDemoActionListViewController

- (instancetype)init {
    self = [super initWithNibName:nil bundle:nil];
    if (self) {
        _actionsEnabled = YES;
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemGroupedBackgroundColor;
    [self buildSubviews];
    [self reloadActions];
}

- (void)buildSubviews {
    _tableView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
    _tableView.dataSource = self;
    _tableView.delegate = self;
    _tableView.translatesAutoresizingMaskIntoConstraints = NO;
    _tableView.rowHeight = UITableViewAutomaticDimension;
    _tableView.estimatedRowHeight = 52;
    [_tableView registerClass:VTMDemoActionCell.class forCellReuseIdentifier:kCellID];
    [self.view addSubview:_tableView];

    _console = [[VTMDemoConsoleView alloc] initWithFrame:CGRectZero];
    _console.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:_console];

    [NSLayoutConstraint activateConstraints:@[
        [_tableView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [_tableView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [_tableView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],

        [_console.topAnchor constraintEqualToAnchor:_tableView.bottomAnchor],
        [_console.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [_console.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [_console.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],
        [_console.heightAnchor constraintEqualToAnchor:self.view.heightAnchor multiplier:0.34],
    ]];
}

#pragma mark - 对外

- (NSArray <VTMDemoActionSection *> *)buildSections {
    return @[];
}

- (void)reloadActions {
    self.sections = [self buildSections];
    [self.tableView reloadData];
}

- (void)setActionsEnabled:(BOOL)actionsEnabled {
    _actionsEnabled = actionsEnabled;
    [self.tableView reloadData];
}

- (void)log:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *line = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self.console appendLine:line];
}

#pragma mark - UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return (NSInteger)self.sections.count;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return (NSInteger)self.sections[(NSUInteger)section].actions.count;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    return self.sections[(NSUInteger)section].title;
}

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section {
    return self.sections[(NSUInteger)section].footer;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:kCellID forIndexPath:indexPath];
    VTMDemoAction *action = self.sections[(NSUInteger)indexPath.section].actions[(NSUInteger)indexPath.row];

    cell.textLabel.text = action.title;
    cell.textLabel.font = [UIFont systemFontOfSize:15];
    cell.detailTextLabel.text = action.detail;
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;

    BOOL enabled = self.actionsEnabled;
    cell.textLabel.enabled = enabled;
    cell.selectionStyle = enabled ? UITableViewCellSelectionStyleDefault : UITableViewCellSelectionStyleNone;
    cell.contentView.alpha = enabled ? 1.0 : 0.4;

    return cell;
}

#pragma mark - UITableViewDelegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    if (!self.actionsEnabled) {
        [self log:@"session 还没就绪，先等 -sessionDeployCompletion:"];
        return;
    }
    VTMDemoAction *action = self.sections[(NSUInteger)indexPath.section].actions[(NSUInteger)indexPath.row];
    if (action.handler) {
        action.handler();
    }
}

@end
