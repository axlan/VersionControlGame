#include "HelloWorldScene.h"
#include "GraphNode.h"
#include "SimpleAudioEngine.h"
#include "GameBoard.h"

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


	_graph_node = GraphNode::create();
	_graph_node->setContentSize(cocos2d::Size(800, 200));
	_graph_node->setPosition(cocos2d::Vec2(0, 0));
    this->addChild(_graph_node);

    auto main_board = GameBoard::create();
	main_board->setContentSize(cocos2d::Size(512, 512));
	main_board->setPosition(cocos2d::Vec2(528, 200));
	this->addChild(main_board);

	auto last_board = GameBoard::create();
	last_board->setContentSize(cocos2d::Size(512, 512));
	last_board->setPosition(cocos2d::Vec2(0, 200));
	this->addChild(last_board);

	_graph_node->set_game_boards(main_board, last_board);
	main_board->EnableInput(_graph_node);
	
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
	/*
	cocos2d::log("%d %d\n", pSender->_ID, pSender);
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
*/
	_graph_node->Merge();

    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
    
    
}
