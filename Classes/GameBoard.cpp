/* Copyright (c) 2017 Axlan */

#include <string>
#include <SimpleAudioEngine.h>

#include "GameBoard.h"
#include "GraphNode.h"


USING_NS_CC;

//////////////////////////// Static ////////////////////////////////////////////

static int IntFromPoint(Point point, const TMXLayer* layer) {
    return static_cast<int>(point.x + layer->getLayerSize().width * point.y);
}

static void CopyLayerState(TMXLayer* layer, Map2D* tiles) {
    for (int row = 0; row < layer->getLayerSize().height; row++) {
        for (int col = 0; col < layer->getLayerSize().width; col++) {
            Point pos(col, row);
            int tile_gid = layer->getTileGIDAt(Point(col, row));
            if (tile_gid) {
                tiles->insert({IntFromPoint(pos, layer), tile_gid});
            }
        }
    }
}

static void RestoreLayerState(TMXLayer* layer, const Map2D& tiles) {
    for (int row = 0; row < layer->getLayerSize().height; row++) {
        for (int col = 0; col < layer->getLayerSize().width; col++) {
            Point pos(col, row);
            int tile_gid = layer->getTileGIDAt(Point(col, row));
            int stored_gid;
            int int_pos = IntFromPoint(pos, layer);
            if (tiles.count(int_pos) == 0) {
                stored_gid = 0;
            } else {
                stored_gid = tiles.at(int_pos);
            }
            if (tile_gid != stored_gid) {
                if (!stored_gid) {
                    layer->removeTileAt(pos);
                } else {
                    layer->setTileGID(stored_gid, pos);
                }
            }
        }
    }
}

////////////////////////////////////// TriggerCmd //////////////////////////////

TriggerCmd::TriggerCmd() :type(Type::NONE) {}

TriggerCmd::TriggerCmd(const std::string& cmd_str, TMXLayer* layer) {
    auto words = split(cmd_str, ' ');
    if (words[0].compare("RESTORE") == 0) {
        type = Type::RESTORE;
        arg1 = ParsePoint(words[1]);
        arg2 = layer->getTileGIDAt(arg1);
    } else if (words[0].compare("RM") == 0) {
        type = Type::RM;
        arg1 = ParsePoint(words[1]);
    } else {
        assert(0 && "Invalid trigger cmd");
    }
}

void TriggerCmd::Triggger(TMXLayer* layer) {
    if (type == Type::RM) {
        layer->removeTileAt(arg1);
    } else if (type == Type::RESTORE) {
        layer->setTileGID(arg2, arg1);
    }
}

Point TriggerCmd::ParsePoint(const std::string& word) {
    int idx = word.find(',');
    return Point(atoi(word.c_str()), atoi(word.substr(idx + 1).c_str()));
}


/////////////////////////////////////////////////// GameBoard //////////////////

void GameBoard::RestoreGameState(const GameState& state) {
    this->setPlayerPosition(state.plater_pos);
    RestoreLayerState(_dynamic, state.dynamic_layer);
    RerunTriggers();
}

Point GameBoard::GetTileSize() const {
    Point scale = GetWorldToNodeScale(this);
    float width = _tileMap->getTileSize().width * scale.x;
    float height = _tileMap->getTileSize().height * scale.y;
    return Point(width, height);
}

Point2D GameBoard::BoardPixel2Point(int x, int y) const {
    Point tile_size = GetTileSize();
    int point_x = static_cast<int>(x / tile_size.x);
    int point_y = static_cast<int>(y / tile_size.y);
    return Point2D{ point_x, point_y };
}

// on "init" you need to initialize your instance
bool GameBoard::init() {
    //////////////////////////////
    // 1. super init first
    if (!Node::init()) {
        return false;
    }
    _graph_node = nullptr;
    _merging = false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();



    _tileMap = new TMXTiledMap();
    _tileMap->initWithTMXFile("test1.tmx");
    _background = _tileMap->getLayer("Background");
    _foreground = _tileMap->getLayer("Foreground");
    _dynamic = _tileMap->getLayer("Dynamic");

    // all tiles are aliased by default, let's set them anti-aliased
    for (const auto& child : _tileMap->getChildren()) {
        static_cast<SpriteBatchNode*>(child)->getTexture()->
            setAntiAliasTexParameters();
    }

    this->addChild(_tileMap);


    TMXObjectGroup* meta_objects = _tileMap->getObjectGroup("Meta");

    for (const auto &object : meta_objects->getObjects()) {
        const ValueMap map = object.asValueMap();
        Point2D pos = BoardPixel2Point(map.at("x").asInt(),
                                       map.at("y").asInt());
        uint64_t key = Point2Key(pos);
        if (map.count("enter_trigger")) {
            _enter_trigger[key] = TriggerCmd(
                map.at("enter_trigger").asString(),
                _dynamic);
        }
        if (map.count("exit_trigger")) {
            _exit_trigger[key] = TriggerCmd(
                map.at("exit_trigger").asString(),
                _dynamic);
        }
    }

    _numCollected = 0;

    ValueMap spawnPoint = meta_objects->getObject("SpawnPoint");
    Point2D start_pos = BoardPixel2Point(spawnPoint["x"].asInt(),
                                         spawnPoint["y"].asInt());


    _player = new Sprite();
    _player->initWithFile("Player.png");
    setPlayerPosition(start_pos);

    this->addChild(_player);
    this->setViewPointCenter(_player->getPosition());


    RerunTriggers();

    boundLines = DrawNode::create();
    this->addChild(boundLines);

    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(GameBoard::on_touch_down, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    return true;
}

void GameBoard::EnableInput(GraphNode* graph_node) {
    _graph_node = graph_node;
    GameState game_state;
    game_state.plater_pos = _player_pos;
    CopyLayerState(_dynamic, &game_state.dynamic_layer);
    _graph_node->UpdateGameState(game_state);

    DrawBounds(Color4F::MAGENTA);

    auto eventListener = EventListenerKeyboard::create();
    eventListener->onKeyPressed = CC_CALLBACK_2(GameBoard::on_key_down, this);
    this->_eventDispatcher->addEventListenerWithFixedPriority(eventListener, 1);
}

bool GameBoard::IsTileInbounds(const Point2D& position) {
    // safety check on the bounds of the map
    return (position.x < (_tileMap->getMapSize().width) &&
        position.y < (_tileMap->getMapSize().height) &&
        position.y >= 0 &&
        position.x >= 0);
}

void GameBoard::on_key_down(EventKeyboard::KeyCode keyCode, Event* event) {
    if (_merging) {
        return;
    }
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
    if (IsTileInbounds(new_pos)) {
        if (new_pos != _player_pos) {
            this->setPlayerPosition(new_pos);
        }
        GameState game_state;
        game_state.plater_pos = _player_pos;
        CopyLayerState(_dynamic, &game_state.dynamic_layer);
        _graph_node->AddGameState(game_state);
    }

    this->setViewPointCenter(_player->getPosition());
}

bool GameBoard::on_touch_down(Touch* touch, Event* event) {

    auto bounds = event->getCurrentTarget()->getBoundingBox();

    if (!bounds.containsPoint(touch->getLocation())) {
        return true;
    }
    Point local_touch = event->getCurrentTarget()->convertToNodeSpace(touch->getLocation());
    //log ("point X=%d Y=%d", local_touch.x, local_touch.y);
    Point2D tile_touch = BoardPixel2Point(local_touch.x, local_touch.y);
    //log("tile X=%d Y=%d", tile_touch.x, tile_touch.y);

    if (!_merging) {
        if (!_graph_node) {
            return true;
        }
        Point2D diff = _player_pos - tile_touch;
        //log("diff X=%d Y=%d", diff.x, diff.y);
        if (diff == Point2D(0, 0)) {
            return true;
        }
        if (abs(diff.x) > abs(diff.y)) {
            if (diff.x > 0) {
                on_key_down(EventKeyboard::KeyCode::KEY_LEFT_ARROW, nullptr);
            } else {
                on_key_down(EventKeyboard::KeyCode::KEY_RIGHT_ARROW, nullptr);
            }
        } else {
            if (diff.y > 0) {
                on_key_down(EventKeyboard::KeyCode::KEY_DOWN_ARROW, nullptr);
            } else {
                on_key_down(EventKeyboard::KeyCode::KEY_UP_ARROW, nullptr);
            }
        }
    } else {
        bool done = true;

        for (auto &tile : _merge_tiles) {
            if (tile_touch == tile.position) {
                tile.selected = MergeTile::SelectedBy::SELF;
                _merge_pair->SelectedOtherMergeCandidate(tile.gid);
                DrawMergeTile(tile);
            } else if (tile.selected == MergeTile::SelectedBy::NEITHER) {
                done = false;
            }
        }

        if (done) {
            GraphNode* graph_node = (_graph_node) ? _graph_node :
                _merge_pair->_graph_node;
            GameState game_state;
            CopyLayerState(_dynamic, &game_state.dynamic_layer);

            for (const auto &tile : _merge_tiles) {
                const MergeTile * chosen = nullptr;
                if (tile.selected == MergeTile::SelectedBy::SELF) {
                    chosen = &tile;
                } else {
                    for (const auto &other_tile : _merge_pair->_merge_tiles) {
                        if (other_tile.gid == tile.gid) {
                            chosen = &other_tile;
                            break;
                        }
                    }
                }
                if (chosen->gid == 0) {
                    game_state.plater_pos = chosen->position;
                } else {
                    if (chosen != &tile) {
                        int map_id = IntFromPoint(getTileCoord(tile.position),
                            _dynamic);
                        game_state.dynamic_layer.erase(map_id);
                    }
                    int map_id = IntFromPoint(getTileCoord(chosen->position),
                        _dynamic);
                    game_state.dynamic_layer[map_id] = chosen->gid;
                }
            }
            graph_node->StopMerge(game_state);
        }
    }
    return true;
}

void GameBoard::SelectedOtherMergeCandidate(int selected_gid) {
    for (auto &tile : _merge_tiles) {
        if (selected_gid == tile.gid) {
            tile.selected = MergeTile::SelectedBy::OTHER;
            DrawMergeTile(tile);
            break;
        }
    }
}

Point GameBoard::getTileCoord(const Point2D& position) const {
    return Point(position.x, _tileMap->getMapSize().height - 1 - position.y);
}

void  GameBoard::StartMerge(GameBoard * other) {
    if (_merging) {
        return;
    }
    _merging = true;
    _merge_pair = other;
    for (int row = 0; row < _dynamic->getLayerSize().height; row++) {
        for (int col = 0; col < _dynamic->getLayerSize().width; col++) {
            Point pos(col, row);
            int tile_gid = _dynamic->getTileGIDAt(Point(col, row));
            if (tile_gid) {
                if (IsAtribute(_tileMap, tile_gid, "Mergeable")) {
                    MergeTile tile(tile_gid, getTileCoord(pos));
                    _merge_tiles.push_back(tile);
                    DrawMergeTile(tile);
                }
            }
        }
    }

    MergeTile player(_player_pos);
    _merge_tiles.push_back(player);
    DrawMergeTile(player);
}

void GameBoard::StopMerge() {
    Color4F color = (_graph_node) ? Color4F::MAGENTA : Color4F::YELLOW;
    DrawBounds(color);
    _merge_tiles.clear();
    _merging = false;
}

void GameBoard::DrawMergeTile(const MergeTile &tile) {
    Color4F color;
    switch (tile.selected) {
    case MergeTile::SelectedBy::NEITHER: color = Color4F::BLUE; break;
    case MergeTile::SelectedBy::SELF: color = Color4F::GREEN; break;
    case MergeTile::SelectedBy::OTHER: color = Color4F::RED; break;
    }


    Point tile_size = GetTileSize();
    Point origin(tile.position.x * tile_size.x,
                 tile.position.y * tile_size.y);
    boundLines->drawRect(origin,
                         origin + Point(tile_size.x, tile_size.y), color);
}

void GameBoard::RerunTriggers() {
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

void GameBoard::CheckEnter(const Point2D& position) {
    uint64_t key = Point2Key(position);
    if (_enter_trigger.count(key)) {
        _enter_trigger[key].Triggger(_dynamic);
    }
}

void GameBoard::CheckExit(const Point2D& position) {
    uint64_t key = Point2Key(position);
    if (_exit_trigger.count(key)) {
        _exit_trigger[key].Triggger(_dynamic);
    }
}

void GameBoard::setPlayerPosition(const Point2D &position) {
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
    } else if (IsAtribute(_tileMap, tile_dynamic_gid, "Solid")) {
        return;
    }

    CheckExit(_player_pos);
    CheckEnter(position);
    _player_pos = position;
    Point tile_size = GetTileSize();
    _player->setPosition(position.x * tile_size.x + tile_size.x / 2,
                         position.y * tile_size.y + tile_size.y / 2);
}

void GameBoard::DrawBounds(cocos2d::Color4F color) {
    boundLines->clear();

    auto bounds = this->getContentSize();
    int max_x = bounds.width - 1;
    int max_y = bounds.height - 1;

    boundLines->drawRect(Vec2(0, 0), Vec2(max_x, max_y), color);
}

void GameBoard::setViewPointCenter(Point position) {
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

void GameBoard::setPositionRect(const cocos2d::Rect &postion) {
    this->setContentSize(postion.size);
    this->setPosition(postion.origin);
    DrawBounds(Color4F::YELLOW);
}
