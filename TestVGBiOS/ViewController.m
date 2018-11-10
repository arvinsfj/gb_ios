//
//  ViewController.m
//  TestNESiOS
//
//  Created by arvin on 8/25/17.
//  Copyright © 2017 arvin. All rights reserved.
//

#import "ViewController.h"

int vmain(int argc, const char* argv);
void wnd_key2btn(int key, char isDown);

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UIImageView *canvasImageView;
@property (weak, nonatomic) IBOutlet UILabel *fpsLabel;

@end

@implementation ViewController

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(video:) name:@"gb_video" object:nil];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSString* path = [[NSBundle mainBundle] pathForResource:@"Tetris" ofType:@"gb"];
        vmain((int)path.length, [path UTF8String]);
    });
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

double time0 = 0; uint8_t fpsArr[100]; uint16_t len = 0;
- (void)calcFPS
{
    double now = CFAbsoluteTimeGetCurrent();
    uint8_t fps = 1/(now - time0);
    time0 = now;
    if (len < 100) {
        fpsArr[len++] = fps;
    }else{
        uint64_t avgFPS = 0;
        for (int i = 0; i < 100; i++) {
            avgFPS+=fpsArr[i];
        }
        avgFPS/=100;
        dispatch_async(dispatch_get_main_queue(), ^{
            self->_fpsLabel.text = [NSString stringWithFormat:@"%d", (uint8_t)avgFPS];
        });
        len = 0;
        fpsArr[len++] = fps;
    }
}

- (void)video:(NSNotification*)n
{
    //子线程
    [self calcFPS];
    UIImage* image = [n.userInfo objectForKey:@"video"];
    dispatch_async(dispatch_get_main_queue(), ^{
        self->_canvasImageView.image = nil;
        self->_canvasImageView.image = image;
    });
}

- (IBAction)btnDown:(id)sender {
    NSInteger tag = ((UIButton*)sender).tag;
    wnd_key2btn((int)tag, YES);
}
- (IBAction)btnUp:(id)sender {
    NSInteger tag = ((UIButton*)sender).tag;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^(void){
        wnd_key2btn((int)tag, NO);
    });
}

@end
