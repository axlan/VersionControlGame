/* Copyright (c) 2017 Axlan */

#ifndef CLASSES_GAMEBOARD_H_
#define CLASSES_GAMEBOARD_H_

#include <cocos2d.h>
#include <unordered_map>
#include <string>
#include <list>

#include "HudLayer.h"
#include "Util.h"

struct TriggerCmd {
    enum class Type {
        RESTORE,
        RM,
        NONE
    };
    Type type;
    Point arg1;
    int arg2;
    TriggerCmd();
    TriggerCmd(const std::string& cmd_str, TMXLayer* layer);

    void Triggger(TMXLayer* layer);
 private:
    Point ParsePoint(const std::string& word);
};

struct MergeTile {
    enum class SelectedBy {
        NEITHER,
        SELF,
        OTHER
    };
    int gid;
    Point2D position;
    SelectedBy selected;

    MergeTile() :
        gid(0),
        position(Point(0, 0)),
        selected(SelectedBy::NEITHER) {}
    explicit MergeTile(const Point2D &position) :
        gid(0),
        position(position),
        selected(SelectedBy::NEITHER) {}
    MergeTile(int gid, const Point2D &position) :
        gid(gid),
        position(position),
        selected(SelectedBy::NEITHER) {}
};

typedef std::unordered_map<uint64_t, TriggerCmd> TriggerMap;


typedef std::unordered_map<int, int> Map2D;

struct GameState {
    Map2D dynamic_layer;
    Point2D plater_pos;
};

class GraphNode;

class GameBoard : public cocos2d::Node {
 public:
    virtual bool init();
    // implement the "static create()" method manually
    CREATE_FUNC(GameBoard);

    bool on_touch_down(Touch* touch, Event* event);
    void on_key_down(cocos2d::EventKeyboard::KeyCode keyCode,
                     cocos2d::Event* event);

    void EnableInput(GraphNode* graph_node);

    void RestoreGameState(const GameState& state);

    void StartMerge(GameBoard * other);
    void StopMerge();

    void setPositionRect(const cocos2d::Rect &postion);

 private:
    void DrawBounds(cocos2d::Color4F color);

    cocos2d::TMXTiledMap *_tileMap;
    cocos2d::TMXLayer *_background;
    cocos2d::TMXLayer *_foreground;
    cocos2d::TMXLayer *_dynamic;
    TriggerMap _enter_trigger;
    TriggerMap _exit_trigger;
    cocos2d::Sprite *_player;
    HudLayer *_hud;
    int _numCollected;
    Point2D _player_pos;

    void DrawMergeTile(const MergeTile &tile);
    void SelectedOtherMergeCandidate(int selected_gid);

    void CheckEnter(const Point2D& position);
    void CheckExit(const Point2D& position);
    Point2D BoardPixel2Point(int x, int y) const;
    cocos2d::Point GetTileSize() const;
    void setPlayerPosition(const Point2D& position);
    Point getTileCoord(const Point2D& position) const;
    bool IsTileInbounds(const Point2D& position);
    void setViewPointCenter(cocos2d::Point position);
    void RerunTriggers();
    GraphNode* _graph_node;
    cocos2d::DrawNode *boundLines;
    bool _merging;
    GameBoard * _merge_pair;
    std::list<MergeTile> _merge_tiles;
};

#endif    // CLASSES_GAMEBOARD_H_
