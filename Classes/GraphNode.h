/* Copyright (c) 2017 Axlan */

#ifndef CLASSES_GRAPHNODE_H_
#define CLASSES_GRAPHNODE_H_

#include <cocos2d.h>
#include <unordered_map>
#include <memory>

class TestNode;
typedef std::shared_ptr<TestNode> TestNodePtr;

struct GameState;
class GameBoard;

enum class ClickType
{
    SELECT,
    DELETE,
    VIEW
};


class GraphNode : public cocos2d::Node {
 public:
    virtual bool init();
    // implement the "static create()" method manually
    CREATE_FUNC(GraphNode);

    bool on_touch_down(cocos2d::Touch* touch, cocos2d::Event* event);

    void AddGameState(const GameState& state);
    void UpdateGameState(const GameState& state);

    void set_game_boards(GameBoard* current, GameBoard* last);

    void StartMerge();
    void StopMerge(const GameState& state);

    void set_click(ClickType click);

 private:
    void update_nodes();
    cocos2d::DrawNode *boundLines;
    TestNodePtr root;
    std::unordered_map<uint64_t, TestNodePtr> point_map;

    TestNodePtr _selected;
    TestNodePtr _prev_selected;

    GameBoard* _current_game_board;
    GameBoard* _last_game_board;
    bool _merging;
    ClickType _click;
};

#endif    // CLASSES_GRAPHNODE_H_
