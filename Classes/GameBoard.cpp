#include "GameBoard.h"
#include "SimpleAudioEngine.h"
#include "GraphNode.h"

USING_NS_CC;

////////////////////////////Static///////////////////////////////////////////////////

static int IntFromPoint(Point point, const TMXLayer* layer)
{
	return static_cast<int>(point.x + layer->getLayerSize().width * point.y);
}


static void CopyLayerState(TMXLayer* layer, Map2D& tiles) {
	for (int row = 0; row < layer->getLayerSize().height; row++) {
		for (int col = 0; col < layer->getLayerSize().width; col++) {
			Point pos(col, row);
			int tile_gid = layer->getTileGIDAt(Point(col, row));
			if (tile_gid) {
				tiles[IntFromPoint(pos, layer)] = tile_gid;
			}
		}
	}
}


static void RestoreLayerState(TMXLayer* layer, Map2D& tiles) {
	for (int row = 0; row < layer->getLayerSize().height; row++) {
		for (int col = 0; col < layer->getLayerSize().width; col++) {
			Point pos(col, row);
			int tile_gid = layer->getTileGIDAt(Point(col, row));
			int stored_gid;
			int int_pos = IntFromPoint(pos, layer);
			if (tiles.count(int_pos) == 0) {
				stored_gid = 0;
			}
			else {
				stored_gid = tiles[int_pos];
			}
			if (tile_gid != stored_gid) {
				if (!stored_gid) {
					layer->removeTileAt(pos);
				}
				else {
					layer->setTileGID(stored_gid, pos);
				}
			}
		}
	}
}

//////////////////////////////////////TriggerCmd///////////////////////////////////////////////////

TriggerCmd::TriggerCmd():type(Type::NONE) {}

TriggerCmd::TriggerCmd(const std::string& cmd_str, TMXLayer* layer)
{
  auto words = split(cmd_str, ' ');
  if (words[0].compare("RESTORE") == 0) {
    type = Type::RESTORE;
    arg1 = ParsePoint(words[1]);
    arg2 = layer->getTileGIDAt(arg1);
  }
  else if (words[0].compare("RM") == 0) {
    type = Type::RM;
    arg1 = ParsePoint(words[1]);
  }
  else {
    assert(0 && "Invalid trigger cmd");
  }
}

void TriggerCmd::Triggger(TMXLayer* layer)
{
  if (type == Type::RM) {
    layer->removeTileAt(arg1);
  }
  else if (type == Type::RESTORE) {
    layer->setTileGID(arg2, arg1);
  }
}

Point TriggerCmd::ParsePoint(const std::string& word)
{
  int idx = word.find(',');
  return Point(atoi(word.c_str()), atoi(word.substr(idx + 1).c_str()));
}


///////////////////////////////////////////////////GameBoard///////////////////////////////////////////////////

void GameBoard::RestoreGameState(GameState& state)
{
	this->setPlayerPosition(state.plater_pos);
	RestoreLayerState(_dynamic, state.dynamic_layer);
	RerunTriggers();
}

Point GameBoard::GetTileSize() const
{
  Point scale = GetWorldToNodeScale(this);
  float width = _tileMap->getTileSize().width * scale.x;
  float height = _tileMap->getTileSize().height * scale.y;
  return Point(width, height);
}

Point2D GameBoard::BoardPixel2Point(int x, int y) const
{
  Point tile_size = GetTileSize();
  return Pixel2Point(x, y, 0, 0, tile_size.x, tile_size.y);
}

// on "init" you need to initialize your instance
bool GameBoard::init()
{

  //////////////////////////////
  // 1. super init first
  if (!Node::init())
  {
    return false;
  }
  _graph_node = nullptr;

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();



  _tileMap = new TMXTiledMap();
  _tileMap->initWithTMXFile("test1.tmx");
  _background = _tileMap->getLayer("Background");
  _foreground = _tileMap->getLayer("Foreground");
  _dynamic = _tileMap->getLayer("Dynamic");

  // all tiles are aliased by default, let's set them anti-aliased
  for (const auto& child : _tileMap->getChildren())
  {
    static_cast<SpriteBatchNode*>(child)->getTexture()->setAntiAliasTexParameters();
  }

  this->addChild(_tileMap);


  TMXObjectGroup* meta_objects = _tileMap->getObjectGroup("Meta");

  for (const auto &object : meta_objects->getObjects()) {
    const ValueMap map = object.asValueMap();
    Point2D pos = BoardPixel2Point(map.at("x").asInt(), map.at("y").asInt());
    uint64_t key = Point2Key(pos);
    if (map.count("enter_trigger")) {
      _enter_trigger[key] = TriggerCmd(map.at("enter_trigger").asString(), _dynamic);
    }
    if (map.count("exit_trigger")) {
      _exit_trigger[key] = TriggerCmd(map.at("exit_trigger").asString(), _dynamic);
    }
  }

  _numCollected = 0;

  ValueMap spawnPoint = meta_objects->getObject("SpawnPoint");
  Point2D start_pos = BoardPixel2Point(spawnPoint["x"].asInt(), spawnPoint["y"].asInt());


  _player = new Sprite();
  _player->initWithFile("Player.png");
  setPlayerPosition(start_pos);

  this->addChild(_player);
  this->setViewPointCenter(_player->getPosition());


  RerunTriggers();

  boundLines = DrawNode::create();
  this->addChild(boundLines);

  /*
  auto listener = EventListenerMouse::create();
  listener->onMouseDown = CC_CALLBACK_1(GameBoard::on_mouse_down, this);
  _eventDispatcher->addEventListenerWithFixedPriority(listener, 1);
  */



  return true;
}

void GameBoard::EnableInput(GraphNode* graph_node)
{
	_graph_node = graph_node;
	GameState game_state;
	game_state.plater_pos = _player_pos;
	CopyLayerState(_dynamic, game_state.dynamic_layer);
	_graph_node->UpdateGameState(game_state);

	DrawBounds(Color4F(1.0, 0.0, 1.0, 1.0));

	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = CC_CALLBACK_2(GameBoard::on_key_down, this);
	this->_eventDispatcher->addEventListenerWithFixedPriority(eventListener, 1);

	auto listener = EventListenerMouse::create();
	listener->onMouseDown = CC_CALLBACK_1(GameBoard::on_mouse_down, this);
	listener->onMouseMove = [](cocos2d::Event* event) { };
	listener->onMouseScroll = [](cocos2d::Event* event) {};
	listener->onMouseUp = [](cocos2d::Event* event) { };
	_eventDispatcher->addEventListenerWithFixedPriority(listener, 1);
}

void GameBoard::on_mouse_down(cocos2d::Event* event) {
  try {
    EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
    mouseEvent->getMouseButton();
    std::stringstream message;

    Point touchLocation = mouseEvent->getLocationInView();
    //touchLocation = CCDirector::sharedDirector()->convertToGL(touchLocation);
   
    touchLocation = convertToNodeSpace(touchLocation);
    log(" TouchLocation X=%f TouchLocation Y=%f", touchLocation.x, touchLocation.y);

    Point2D point = BoardPixel2Point(touchLocation.x, touchLocation.y);

    log(" point X=%d point Y=%d", point.x, point.y);

    log("w %f h %f mx %f mh %f", _tileMap->getTileSize().width, _tileMap->getTileSize().height, this->getContentSize().width, this->getContentSize().height);

  }
  catch (std::bad_cast& e) {
    log(e.what());
    // Not sure what kind of event you passed us cocos, but it was the wrong one
    return;
  }
}

bool GameBoard::IsTileInbounds(const Point2D& position)
{
  // safety check on the bounds of the map
  return (position.x < (_tileMap->getMapSize().width) &&
    position.y < (_tileMap->getMapSize().height) &&
    position.y >= 0 &&
    position.x >= 0);
}



void GameBoard::on_key_down(EventKeyboard::KeyCode keyCode, Event* event)
{
  Point2D new_pos = _player_pos;

  switch (keyCode) {
  case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
  case EventKeyboard::KeyCode::KEY_A:
    new_pos.x--;
    break;
  case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
  case EventKeyboard::KeyCode::KEY_D:
    new_pos.x++;
    break;
  case EventKeyboard::KeyCode::KEY_UP_ARROW:
  case EventKeyboard::KeyCode::KEY_W:
    new_pos.y++;
    break;
  case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
  case EventKeyboard::KeyCode::KEY_S:
    new_pos.y--;
    break;
  case EventKeyboard::KeyCode::KEY_SPACE:
	  break;
  default:
    return;
  }
  // safety check on the bounds of the map
  if (IsTileInbounds(new_pos))
  {
	if (new_pos != _player_pos) {
		this->setPlayerPosition(new_pos);
	}
	GameState game_state;
	game_state.plater_pos = _player_pos;
	CopyLayerState(_dynamic, game_state.dynamic_layer);
	_graph_node->AddGameState(game_state);
  }

  this->setViewPointCenter(_player->getPosition());
}
/*
void GameBoard::on_mouse_down(cocos2d::Event* event)
{
EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
Point touchLocation = mouseEvent->getLocationInView();
touchLocation = Director::sharedDirector()->convertToGL(touchLocation);
touchLocation = this->convertToNodeSpace(touchLocation);

Point playerPos = _player->getPosition();
Point diff = touchLocation - playerPos;

if (abs(diff.x) > abs(diff.y)) {
if (diff.x > 0) {
playerPos.x += _tileMap->getTileSize().width;
}
else {
playerPos.x -= _tileMap->getTileSize().width;
}
}
else {
if (diff.y < 0) {
playerPos.y += _tileMap->getTileSize().height;
}
else {
playerPos.y -= _tileMap->getTileSize().height;
}
}

// safety check on the bounds of the map
if (playerPos.x <= (_tileMap->getMapSize().width * _tileMap->getTileSize().width) &&
playerPos.y <= (_tileMap->getMapSize().height * _tileMap->getTileSize().height) &&
playerPos.y >= 0 &&
playerPos.x >= 0)
{
this->setPlayerPosition(playerPos);
}

this->setViewPointCenter(_player->getPosition());

}
*/

Point GameBoard::getTileCoord(const Point2D& position) const
{
  return Point(position.x, _tileMap->getMapSize().height - 1 - position.y);
}


void GameBoard::RerunTriggers()
{
	for (auto trigger : _exit_trigger) {
		trigger.second.Triggger(_dynamic);
	}
	for (int row = 0; row < _dynamic->getLayerSize().height; row++) {
		for (int col = 0; col < _dynamic->getLayerSize().width; col++) {
			Point2D position = Point2D(col, row);
			Point tile_coord = getTileCoord(position);
			int gid = _dynamic->getTileGIDAt(tile_coord);
			if (IsAtribute(_tileMap, gid, "Solid")) {
				CheckEnter(position);
			}
		}
	}
	CheckEnter(_player_pos);
}

void GameBoard::CheckEnter(const Point2D& position)
{
  uint64_t key = Point2Key(position);
  if (_enter_trigger.count(key)) {
    _enter_trigger[key].Triggger(_dynamic);
  }
}

void GameBoard::CheckExit(const Point2D& position)
{
  uint64_t key = Point2Key(position);
  if (_exit_trigger.count(key)) {
    _exit_trigger[key].Triggger(_dynamic);
  }
}

void GameBoard::setPlayerPosition(const Point2D &position)
{
  Point tile_coord = getTileCoord(position);
  int tile_foreground_gid = _foreground->getTileGIDAt(tile_coord);
  if (IsAtribute(_tileMap, tile_foreground_gid, "Solid")) {
    return;
  }

  int tile_dynamic_gid = _dynamic->getTileGIDAt(tile_coord);
  if (IsAtribute(_tileMap, tile_dynamic_gid, "Push")) {
    Point2D delta = position - _player_pos;
    Point2D next = position + delta;
    if (!IsTileInbounds(next)) {
      return;
    }
    Point next_tile_coord = getTileCoord(next);
    int push_foreground_gid = _foreground->getTileGIDAt(next_tile_coord);
    if (IsAtribute(_tileMap, push_foreground_gid, "Solid")) {
      return;
    }
    int push_dynamic_gid = _dynamic->getTileGIDAt(next_tile_coord);
    if (IsAtribute(_tileMap, push_dynamic_gid, "Solid")) {
      return;
    }
    CheckExit(position);
    CheckEnter(next);
    _dynamic->removeTileAt(tile_coord);
    _dynamic->setTileGID(tile_dynamic_gid, next_tile_coord);
  }
  else if (IsAtribute(_tileMap, tile_dynamic_gid, "Solid")) {
    return;
  }

  CheckExit(_player_pos);
  CheckEnter(position);
  _player_pos = position;
  Point tile_size = GetTileSize();
  _player->setPosition(position.x * tile_size.x + tile_size.x / 2, position.y * tile_size.y + tile_size.y / 2);

}

void GameBoard::DrawBounds(cocos2d::Color4F color)
{
	boundLines->clear();

	auto bounds = this->getContentSize();
	int max_x = bounds.width - 1;
	int max_y = bounds.height - 1;

	boundLines->drawLine(Vec2(0, 0), Vec2(0, max_y), color);
	boundLines->drawLine(Vec2(0, 0), Vec2(max_x, 0), color);
	boundLines->drawLine(Vec2(max_x, max_y), Vec2(0, max_y), color);
	boundLines->drawLine(Vec2(max_x, max_y), Vec2(max_x, 0), color);
}

void GameBoard::setViewPointCenter(Point position)
{
  /*
  Size winSize = this->getContentSize();

  int x = MAX(position.x, winSize.width / 2);
  int y = MAX(position.y, winSize.height / 2);
  x = MIN(x, (_tileMap->getMapSize().width * this->_tileMap->getTileSize().width) - winSize.width / 2);
  y = MIN(y, (_tileMap->getMapSize().height * _tileMap->getTileSize().height) - winSize.height / 2);
  Point actualPosition = Vec2(x, y);

  Point centerOfView = Vec2(winSize.width / 2, winSize.height / 2);
  Point viewPoint = centerOfView - actualPosition;
  this->setPosition(viewPoint);
  */
}