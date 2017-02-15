#include "HelloWorldScene.h"
#include "GraphNode.h"
#include "SimpleAudioEngine.h"


USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}
 
void HelloWorld::setViewPointCenter(cocos2d::Vec2 position) {

  Size winSize = CCDirector::getInstance()->getWinSize();

  int x = MAX(position.x, winSize.width / 2);
  int y = MAX(position.y, winSize.height / 2);
  x = MIN(x, (_tileMap->getMapSize().width * this->_tileMap->getTileSize().width) - winSize.width / 2);
  y = MIN(y, (_tileMap->getMapSize().height * _tileMap->getTileSize().height) - winSize.height / 2);
  Point actualPosition = Vec2(x, y);

  Point centerOfView = Vec2(winSize.width / 2, winSize.height / 2);
  Point viewPoint = centerOfView - actualPosition;
  this->setPosition(viewPoint);
}
// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

	cocos2d::log("%d %d\n", closeItem->_ID, closeItem);
    
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World!", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // position the sprite on the center of the screen
    //sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    //this->addChild(sprite, 0);

    myLine = DrawNode::create();
    this->addChild(myLine);

    auto myGraph = GraphNode::create();

    myGraph->setContentSize(cocos2d::Size(800, 200));
    myGraph->setPosition(cocos2d::Vec2(0, 0));

    this->addChild(myGraph);

    lineLen = 0;

    // create a TMX map
    _tileMap = TMXTiledMap::create("test1.tmx");
    _tileMap->setContentSize(cocos2d::Size(512, 512));
    _tileMap->setScale(2);
    _tileMap->setPosition(cocos2d::Vec2(0, 300));
    addChild(_tileMap);
    // all tiles are aliased by default, let's set them anti-aliased
    for (const auto& child : _tileMap->getChildren())
    {
      static_cast<SpriteBatchNode*>(child)->getTexture()->setAntiAliasTexParameters();
    }

    auto objects = _tileMap->getObjectGroup("objects");
    cocos2d::ValueMap spawn = objects->getObject("SpawnPoint");
    int x = spawn["x"].asInt();
    int y = spawn["y"].asInt();
    log("spawn: %f %f", x, y);
    auto start = _tileMap->convertToWorldSpace(cocos2d::Vec2(x, y));
    log("win: %f %f", start.x, start.y);

    

    auto sprite = Sprite::create("HelloWorld.png");
    sprite->setPosition(start);
    sprite->setScale(.2f);
    this->addChild(sprite, 0);
    //this->setViewPointCenter(sprite->getPosition());

      /*
    int x = spawn
    int y = [spawnPoint[@"y"] integerValue];

    _player = [CCSprite spriteWithFile : @"Player.png"];
      _player.position = Vec2(x, y);

    [self addChild : _player];
    [self setViewPointCenter : _player.position];
    */

	this->scheduleUpdate();
    
    return true;
}

void HelloWorld::update(float delta) {
	lineLen += 10 * delta;
	if (lineLen > 100) {
		myLine->clear();
		lineLen = 0;
	}
	myLine->drawLine(Vec2(200, 200), Vec2(200, 200 + lineLen), Color4F(1.0, 1.0, 1.0, 1.0));
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{/*
	cocos2d::log("%d %d\n", pSender->_ID, pSender);
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
*/

// add a "close" icon to exit the progress. it's an autorelease object
	auto closeItem = MenuItemImage::create(
		"CloseNormal.png",
		"CloseSelected.png",
		CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));


	closeItem->setPosition(Vec2(dynamic_cast<MenuItemImage*>(pSender)->getPositionX()-50, dynamic_cast<MenuItemImage*>(pSender)->getPositionY()));

	// create menu, it's an autorelease object
	auto menu = Menu::create(closeItem, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}
