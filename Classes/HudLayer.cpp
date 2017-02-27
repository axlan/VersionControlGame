#include "HudLayer.h"

using namespace cocos2d;

bool HudLayer::init()
{
    if (Layer::init()) {
        Size winSize = Director::getInstance()->getWinSize();
        
        _label = Label::createWithTTF("0", "fonts/arial.ttf", 18.0);
        _label->setColor(Color3B(0,0,0));
        
        int margin = 10;
        _label->setPosition(Vec2(winSize.width - (_label->getContentSize().width/2) - margin, _label->getContentSize().height/2 + margin));
        this->addChild(_label);
    }
    
    return true;
}

void HudLayer::numCollectedChanged(int numCollected)
{
    std::string labelCollected = std::to_string(numCollected);
    _label->setString(labelCollected.c_str());
}