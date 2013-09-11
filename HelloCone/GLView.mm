//
//  GLView.m
//  HelloArrow
//
//  Created by neo on 13-8-31.
//  Copyright (c) 2013年 neo. All rights reserved.
//

#import "GLView.h"
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES2/gl.h> //  <-- for GL_RENDERBUFFER only
#import <mach/mach_time.h>


@implementation GLView

#pragma mark - init & dealloc
- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)super.layer;
        eaglLayer.opaque = YES;//自己控制 透明度等属性（默认由Quartz控制）， opengl很好控制
		
		
//        //初始化 context 并设置 opengl api 的版本
//        self.glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
//        if ((! self.glContext) || (! [EAGLContext setCurrentContext:self.glContext]))  {
//            self = nil;
//            return nil;
//        }
//        //initialize for opengl
//
//      
//        //1. 配置相关缓冲区
//        self.renderEngine = CreateRenderer1();
		
		EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
		self.glContext = [[EAGLContext alloc] initWithAPI:api];
		if (!self.glContext) {
			api = kEAGLRenderingAPIOpenGLES1;
            self.glContext = [[EAGLContext alloc] initWithAPI:api];
		}
		
		if (!self.glContext || ![EAGLContext setCurrentContext:self.glContext]) {
			self = nil;
			return nil;
		}
		
		if (api == kEAGLRenderingAPIOpenGLES1) {
			NSLog(@"Using OpenGL ES 1.1");
			self.renderEngine = CreateRenderer1();
		} else {
			NSLog(@"Using OpenGL ES 2.0");
			self.renderEngine = CreateRenderer2();
		}
		
		
		
        self.renderEngine->Initalize(CGRectGetWidth(frame), CGRectGetHeight(frame));
		
		//2. 根据配置分配存储空间
        [self.glContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
      
        [self drawView:nil];
        
        self.timeStamp = CACurrentMediaTime();
        CADisplayLink *displayLink = nil;
        displayLink = [CADisplayLink displayLinkWithTarget:self
                                                  selector:@selector(drawView:)];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop]
                          forMode:NSDefaultRunLoopMode];
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(didRotate:)
                                                     name:UIDeviceOrientationDidChangeNotification
                                                   object:nil];
    }
    return self;
}

- (void)dealloc
{
    if ([EAGLContext currentContext] == self.glContext) {
        [EAGLContext setCurrentContext:nil];
    }
    self.glContext = nil;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

//覆写该方法， 因为 super 会调用此方法（delagate）来生成 layer
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}


#pragma mark - draw
//这里相当于原ios里的drawRect, 但更提升一层性能（缓冲操作）
- (void)drawView: (CADisplayLink *)displayLink
{
//    [self.glContext presentRenderbuffer:GL_RENDERBUFFER_OES];
    if (displayLink != nil) {
        float elapsedSeconds = displayLink.timestamp - self.timeStamp;
        self.timeStamp = displayLink.timestamp;
        self.renderEngine->UpdateAnimation(elapsedSeconds);
    }
    self.renderEngine->Render();
    [self.glContext presentRenderbuffer:GL_RENDERBUFFER];
    
}
- (void)didRotate:(NSNotification *)notification
{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    self.renderEngine->OnRotate((DeviceOrientation)orientation);
    [self drawView:nil];
	
}


@end
