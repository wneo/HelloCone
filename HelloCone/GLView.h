//
//  GLView.h
//  HelloArrow
//
//  Created by neo on 13-8-31.
//  Copyright (c) 2013年 neo. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import "IRenderingEngine.hpp"

@interface GLView : UIView
@property (nonatomic, strong) EAGLContext *glContext;
@property (nonatomic) struct IRenderingEngine *renderEngine;
@property (nonatomic) float timeStamp;

- (void)drawView: (CADisplayLink *)displayLink;
- (void)didRotate:(NSNotification *)notification;
@end
