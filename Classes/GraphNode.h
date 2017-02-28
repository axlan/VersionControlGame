#pragma once

#include "cocos2d.h"

class TestNode;
typedef std::shared_ptr<TestNode> TestNodePtr;

struct GameState;

class GraphNode : public cocos2d::Node
{
public:

	virtual bool init();
  
	// implement the "static create()" method manually
	CREATE_FUNC(GraphNode);

  virtual void on_mouse_down(cocos2d::Event* event);

  void AddGameState(const GameState& state);
  void UpdateGameState(const GameState& state);

private:
  void update_nodes();
  cocos2d::DrawNode *boundLines;
  TestNodePtr root;
  std::unordered_map<uint64_t, TestNodePtr> point_map;

  TestNodePtr _selected;

};
