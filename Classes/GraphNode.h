#pragma once

#include "cocos2d.h"

class TestNode;
typedef std::shared_ptr<TestNode> TestNodePtr;

struct GameState;
class GameBoard;

class GraphNode : public cocos2d::Node
{
public:

	virtual bool init();
  
	// implement the "static create()" method manually
	CREATE_FUNC(GraphNode);

  virtual void on_mouse_down(cocos2d::Event* event);

  void AddGameState(const GameState& state);
  void UpdateGameState(const GameState& state);

  void set_game_boards(GameBoard* current, GameBoard* last);

  void Merge();

private:
  void update_nodes();
  cocos2d::DrawNode *boundLines;
  TestNodePtr root;
  std::unordered_map<uint64_t, TestNodePtr> point_map;

  TestNodePtr _selected;
  TestNodePtr _prev_selected;

  GameBoard* _current_game_board;
  GameBoard* _last_game_board;



};
