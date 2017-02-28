#pragma once

#include "cocos2d.h"
#include "HudLayer.h"
#include "Util.h"

struct TriggerCmd
{
  enum class Type
  {
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

typedef std::unordered_map<uint64_t, TriggerCmd> TriggerMap;


typedef std::unordered_map<int, int> Map2D;

struct GameState
{
	Map2D dynamic_layer;
	Point2D plater_pos;
};


class GameBoard : public cocos2d::Node
{
public:

  virtual bool init();
  // implement the "static create()" method manually
  CREATE_FUNC(GameBoard);

  void on_mouse_down(cocos2d::Event* event);
  //virtual void on_mouse_down(cocos2d::Event* event);
  void on_key_down(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

private:
  GameState _game_state;
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

  void CheckEnter(const Point2D& position);
  void CheckExit(const Point2D& position);
  Point2D BoardPixel2Point(int x, int y) const;
  cocos2d::Point GetTileSize() const;
  void setPlayerPosition(const Point2D& position);
  Point getTileCoord(const Point2D& position) const;
  bool IsTileInbounds(const Point2D& position);
  void setViewPointCenter(cocos2d::Point position);
  void RerunTriggers();
};
