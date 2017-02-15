#pragma once

#include "cocos2d.h"

class GameBoard : public cocos2d::Node
{
public:

  virtual bool init();
  void update(float) override;
  // implement the "static create()" method manually
  CREATE_FUNC(GameBoard);

  virtual void on_mouse_down(cocos2d::Event* event);

private:
  cocos2d::DrawNode *boundLines;
  bool need_update;

};
