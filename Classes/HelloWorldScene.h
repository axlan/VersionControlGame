#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
	void update(float) override;
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
    void setViewPointCenter(cocos2d::Vec2 position);
private:
	cocos2d::DrawNode *myLine;
	double lineLen;
  cocos2d::TMXTiledMap *_tileMap;

};

#endif // __HELLOWORLD_SCENE_H__
