#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <ui/UICheckBox.h>

class GraphNode;

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
private:
	GraphNode* _graph_node;

    void check_box_callback(cocos2d::Ref* caller, cocos2d::ui::CheckBox::EventType event);

    cocos2d::ui::CheckBox* check_box_sel;
    cocos2d::ui::CheckBox* check_box_del;
    cocos2d::ui::CheckBox* check_box_view;

};

#endif // __HELLOWORLD_SCENE_H__
