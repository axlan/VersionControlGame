#include "HelloWorldScene.h"
#include "GraphNode.h"
#include "SimpleAudioEngine.h"
#include "GameBoard.h"
#include <ui/UIText.h>
#include <ui/UIButton.h>
#include <ui/UIWidget.h>

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

	_graph_node = GraphNode::create();
	_graph_node->setContentSize(cocos2d::Size(800, 200));
	_graph_node->setPosition(cocos2d::Vec2(0, 0));
    this->addChild(_graph_node);

    auto main_board = GameBoard::create();
	main_board->setPositionRect(Rect(528, 200, 512, 512));
	this->addChild(main_board);

	auto last_board = GameBoard::create();
	last_board->setPositionRect(Rect(0, 200, 512, 512));
	this->addChild(last_board);

	_graph_node->set_game_boards(main_board, last_board);
	main_board->EnableInput(_graph_node);


    Point offset = Point(832, 20);
    check_box_sel = ui::CheckBox::create("check_en_1_sel_0.png",
        "check_en_1_sel_0.png",
        "check_en_1_sel_1.png",
        "check_en_0_sel_0.png",
        "check_en_0_sel_1.png");
    check_box_sel->setPosition(offset);
    check_box_sel->addEventListener(CC_CALLBACK_2(HelloWorld::check_box_callback, this));
    this->addChild(check_box_sel);
    auto label_check_box_sel = ui::Text::create("Select", "fonts/arial.ttf", 30);
    label_check_box_sel->setColor(Color3B::MAGENTA);
    label_check_box_sel->setPosition(offset + Point(60, 0));
    this->addChild(label_check_box_sel);

    check_box_del = ui::CheckBox::create("check_en_1_sel_0.png",
        "check_en_1_sel_0.png",
        "check_en_1_sel_1.png",
        "check_en_0_sel_0.png",
        "check_en_0_sel_1.png");
    check_box_del->setPosition(offset + Point(0, 36));
    check_box_del->addEventListener(CC_CALLBACK_2(HelloWorld::check_box_callback, this));
    this->addChild(check_box_del);
    auto label_check_box_del = ui::Text::create("Delete", "fonts/arial.ttf", 30);
    label_check_box_del->setColor(Color3B::RED);
    label_check_box_del->setPosition(offset + Point(60, 36));
    this->addChild(label_check_box_del);

    check_box_view = ui::CheckBox::create("check_en_1_sel_0.png",
        "check_en_1_sel_0.png",
        "check_en_1_sel_1.png",
        "check_en_0_sel_0.png",
        "check_en_0_sel_1.png");
    check_box_view->setPosition(offset + Point(0, 36 * 2));
    check_box_view->addEventListener(CC_CALLBACK_2(HelloWorld::check_box_callback, this));
    this->addChild(check_box_view);
    auto label_check_box_view = ui::Text::create("View", "fonts/arial.ttf", 30);
    label_check_box_view->setColor(Color3B::YELLOW);
    label_check_box_view->setPosition(offset + Point(55, 36 * 2));
    this->addChild(label_check_box_view);
    check_box_view->setSelected(true);


    auto uButton = ui::Button::create();
    uButton->setTouchEnabled(true);
    uButton->loadTextures("CloseNormal.png", "CloseSelected.png", "");
    uButton->setPosition(offset + Point(0, 36 * 4));
    uButton->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED)
        {
            _graph_node->StartMerge();
        }
    });
    this->addChild(uButton);
    auto label_btn_merge = ui::Text::create("Merge", "fonts/arial.ttf", 30);
    label_btn_merge->setPosition(offset + Point(60, 36 * 4));
    this->addChild(label_btn_merge);

	
    return true;
}

void HelloWorld::check_box_callback(cocos2d::Ref* caller, cocos2d::ui::CheckBox::EventType event)
{
    check_box_view->setSelected(caller == check_box_view);
    if (caller == check_box_view) {
        _graph_node->set_click(ClickType::VIEW);
    }
    check_box_del->setSelected(caller == check_box_del);
    if (caller == check_box_del) {
        _graph_node->set_click(ClickType::DELETE);
    }
    check_box_sel->setSelected(caller == check_box_sel);
    if (caller == check_box_sel) {
        _graph_node->set_click(ClickType::SELECT);
    }
}