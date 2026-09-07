//
//  VTMDemoConsoleView.m
//  VTMBLEDemo
//

#import "VTMDemoConsoleView.h"

@interface VTMDemoConsoleView ()

@property (nonatomic, strong) UITextView *textView;
@property (nonatomic, strong) UILabel *titleLabel;
@property (nonatomic, strong) UIButton *clearButton;
@property (nonatomic, strong) NSDateFormatter *formatter;

@end

@implementation VTMDemoConsoleView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        _formatter = [[NSDateFormatter alloc] init];
        _formatter.dateFormat = @"HH:mm:ss.SSS";

        self.backgroundColor = [UIColor colorWithWhite:0.08 alpha:1.0];
        [self buildSubviews];
    }
    return self;
}

- (void)buildSubviews {
    _titleLabel = [[UILabel alloc] init];
    _titleLabel.text = @"Output";
    _titleLabel.font = [UIFont systemFontOfSize:12 weight:UIFontWeightSemibold];
    _titleLabel.textColor = [UIColor colorWithWhite:0.6 alpha:1.0];
    _titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:_titleLabel];

    _clearButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [_clearButton setTitle:@"清空" forState:UIControlStateNormal];
    _clearButton.titleLabel.font = [UIFont systemFontOfSize:12];
    [_clearButton addTarget:self action:@selector(clear) forControlEvents:UIControlEventTouchUpInside];
    _clearButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:_clearButton];

    _textView = [[UITextView alloc] init];
    _textView.backgroundColor = UIColor.clearColor;
    _textView.textColor = [UIColor colorWithRed:0.65 green:0.90 blue:0.70 alpha:1.0];
    _textView.font = [UIFont monospacedSystemFontOfSize:10 weight:UIFontWeightRegular];
    _textView.editable = NO;
    _textView.alwaysBounceVertical = YES;
    _textView.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:_textView];

    [NSLayoutConstraint activateConstraints:@[
        [_titleLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:12],
        [_titleLabel.topAnchor constraintEqualToAnchor:self.topAnchor constant:6],

        [_clearButton.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-12],
        [_clearButton.centerYAnchor constraintEqualToAnchor:_titleLabel.centerYAnchor],

        [_textView.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [_textView.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [_textView.topAnchor constraintEqualToAnchor:_titleLabel.bottomAnchor constant:4],
        [_textView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
    ]];
}

#pragma mark - 对外

- (void)appendLine:(NSString *)line {
    if (line.length == 0) {
        return;
    }

    if (!NSThread.isMainThread) {
        __weak typeof(self) weakSelf = self;
        dispatch_async(dispatch_get_main_queue(), ^{
            [weakSelf appendLine:line];
        });
        return;
    }

    NSString *stamped = [NSString stringWithFormat:@"%@  %@\n",
                         [self.formatter stringFromDate:NSDate.date], line];
    self.textView.text = [self.textView.text stringByAppendingString:stamped];
    [self scrollToBottom];
}

- (void)appendFormat:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *line = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self appendLine:line];
}

- (void)clear {
    if (!NSThread.isMainThread) {
        __weak typeof(self) weakSelf = self;
        dispatch_async(dispatch_get_main_queue(), ^{ [weakSelf clear]; });
        return;
    }
    self.textView.text = @"";
}

#pragma mark - 私有

- (void)scrollToBottom {
    NSUInteger length = self.textView.text.length;
    if (length == 0) {
        return;
    }
    [self.textView scrollRangeToVisible:NSMakeRange(length - 1, 1)];
}

@end
