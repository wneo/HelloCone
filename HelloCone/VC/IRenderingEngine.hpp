//
//  IRenderingEngine.hpp
//  HelloArrow
//
//  Created by neo on 13-8-31.
//  Copyright (c) 2013年 neo. All rights reserved.
//

#ifndef HelloArrow_IRenderingEngine_hpp
#define HelloArrow_IRenderingEngine_hpp



//物理设备方向
enum DeviceOrientation {
    DeviceOrientationUnknown,
    DeviceOrientationPortrait,
    DeviceOrientationPortraitUpsideDown,
    DeviceOrientationLandscapeLeft,
    DeviceOrientationLandscapeRight,
    DeviceOrientationFaceUp,
    DeviceOrientationFaceDown,
    };

struct IRenderingEngine {
    virtual void Initalize(int width, int height) = 0;
    virtual void Render() const = 0;
    virtual void UpdateAnimation(float timeStep) = 0;
    virtual void OnRotate(DeviceOrientation newOrientation) = 0;
    virtual ~IRenderingEngine() {}
};

struct IRenderingEngine *CreateRenderer1();
struct IRenderingEngine *CreateRenderer2();
#endif
