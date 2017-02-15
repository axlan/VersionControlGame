#pragma once

#include "cocos2d.h"

class TestNode;
typedef std::shared_ptr<TestNode> TestNodePtr;

class GraphNode : public cocos2d::Node
{
public:

	virtual bool init();
  void update(float) override;
	// implement the "static create()" method manually
	CREATE_FUNC(GraphNode);

  virtual void on_mouse_down(cocos2d::Event* event);

private:
	cocos2d::DrawNode *boundLines;
  TestNodePtr root;
  std::unordered_map<uint64_t, TestNodePtr> point_map;
  bool need_update;

};
