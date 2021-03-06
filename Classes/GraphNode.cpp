/* Copyright (c) 2017 Axlan */

#include "GraphNode.h"

#include <assert.h>
#include <list>
#include <vector>
#include <memory>

#include "GameBoard.h"
#include "Util.h"

////////////////// CONSTANTS ///////////////////////////////////////////////////

static const int x_space = 20;
static const int y_space = 20;
static const int radius = 10;
static const int offset = 10;
static const int MAX_BRANCHES = 6;
static const int MAX_STEPS = 40;

////////////////// GRAPH CODE //////////////////////////////////////////////////

class TestNode;

typedef std::shared_ptr<TestNode> TestNodePtr;
typedef std::list<TestNodePtr> TestNodePtrList;

class TestNode {
 public:
    static TestNodePtr MakeRoot(int max_branches, int max_t) {
        return TestNodePtr(new TestNode(max_branches, max_t));
    }

    static TestNodePtr AddChild(TestNodePtr smart_this) {
        auto& branches = *smart_this->branches_;
        uint32_t new_t = smart_this->t_ + 1;
        if (new_t >= static_cast<uint32_t>(smart_this->max_t_) ||
            (branches.size() > new_t &&
             branches[new_t] == smart_this->max_branches_)) {
            return nullptr;
        }
        TestNodePtr child(new TestNode(smart_this));
        smart_this->children_.push_back(child);
        return child;
    }

    static bool CanMerge(TestNodePtr a, TestNodePtr b) {
        if (!a || !b || a->t_ != b->t_) {
            return false;
        }
        auto& branches = *a->branches_;
        uint32_t new_t = a->t_ + 1;
        if (new_t >= a->max_t_ ||
            (branches.size() > new_t &&
             branches[new_t] == a->max_branches_)) {
            return false;
        }
        return true;
    }

    static TestNodePtr Merge(TestNodePtr a, TestNodePtr b) {
        if (!CanMerge(a, b)) {
            return nullptr;
        }
        TestNodePtr child(new TestNode(a));
        child->parent2_ = b;
        a->children_.push_back(child);
        b->children_.push_back(child);
        return child;
    }


    const TestNodePtrList& GetChildren() {
        return children_;
    }

    bool Delete() {
        if (children_.size() > 0 || !parent1_) {
            return false;
        }
        (*branches_)[t_]--;
        parent1_->RemoveChild(this);
        if (parent2_) {
            parent2_->RemoveChild(this);
        }
        return true;
    }

    int branch_id_ = -1;

    std::vector<int> GetParentBranchIDs() {
        std::vector<int> parent_ids;
        parent_ids.push_back(parent1_->branch_id_);
        if (parent2_) {
            parent_ids.push_back(parent2_->branch_id_);
        }
        return parent_ids;
    }

    TestNodePtr GetParent() {
        return parent1_;
    }

    GameState game_state;



 protected:
    void RemoveChild(const TestNode* target) {
        for (auto iterator = this->children_.begin();
             iterator != this->children_.end(); ++iterator) {
            TestNodePtr& child = *iterator;
            if (child.get() == target) {
                this->children_.remove(child);
                return;
            }
        }
        assert(false);
    }

    TestNode(int max_branches, int max_t)
     : max_branches_(max_branches),
       max_t_(max_t),
       parent1_(nullptr),
       parent2_(nullptr),
       t_(0) {
        branches_ = std::make_shared<std::vector<int>>();
        branches_->push_back(1);
    }
    explicit TestNode(TestNodePtr parent)
     : max_branches_(parent->max_branches_),
       max_t_(parent->max_t_),
       parent1_(parent),
       parent2_(nullptr),
       t_(parent->t_ + 1),
       branches_(parent->branches_) {
        auto& branches = *branches_;
        if (branches.size() <= static_cast<uint32_t>(t_)) {
            branches.push_back(1);
        } else {
            branches[t_]++;
        }
    }

    const uint32_t max_branches_;
    const uint32_t max_t_;
    std::shared_ptr<std::vector<int>> branches_;
    TestNodePtr parent1_;
    TestNodePtr parent2_;
    std::list<TestNodePtr> children_;
    int t_;
};



/////////////////////////////////////////////////////////////////////



USING_NS_CC;

void GraphNode::set_game_boards(GameBoard* current, GameBoard* last) {
    _current_game_board = current;
    _last_game_board = last;
}



// on "init" you need to initialize your instance
bool GraphNode::init() {
    //////////////////////////////
    // 1. super init first
    if (!Node::init()) {
        return false;
    }
    _current_game_board = nullptr;
    _last_game_board = nullptr;
    _merging = false;

    _click = ClickType::VIEW;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    boundLines = DrawNode::create();
    this->addChild(boundLines);


    root = TestNode::MakeRoot(MAX_BRANCHES, MAX_STEPS);

    _prev_selected = nullptr;
    _selected = root;

    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(GraphNode::on_touch_down, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    return true;
}


static Point2D Pixel2Point(int x, int y) {
    return Pixel2Point(x, y, offset, offset, x_space, y_space);
}

template <class T>
static bool PtrEqual(T a, T b) {
    return a.get() == b.get();
}


bool GraphNode::on_touch_down(Touch* touch, Event* event) {

    auto bounds = event->getCurrentTarget()->getBoundingBox();

    if (!bounds.containsPoint(touch->getLocation())) {
        return true;
    }
    Point local_touch = event->getCurrentTarget()->convertToNodeSpace(touch->getLocation());
    //log ("point X=%d Y=%d", local_touch.x, local_touch.y);
    Point2D tile_touch = Pixel2Point(local_touch.x, local_touch.y);
    //log("tile X=%d Y=%d", tile_touch.x, tile_touch.y);


    uint64_t key = Point2Key(tile_touch);

    if (point_map.count(key) > 0) {
        if (_click == ClickType::SELECT) {
            if (PtrEqual(point_map[key], _selected)) {
                return true;
            }
            _selected = point_map[key];
            _current_game_board->RestoreGameState(_selected->game_state);
        } else if (_click == ClickType::VIEW) {
            if (PtrEqual(point_map[key], _prev_selected)) {
                return true;
            }
            _prev_selected = point_map[key];
            _last_game_board->RestoreGameState(_prev_selected->game_state);
        } else if (_click == ClickType::DELETE) {
            TestNodePtr target = point_map[key];
            if (PtrEqual(target, _prev_selected)) {
                _prev_selected = nullptr;
            }
            if (PtrEqual(target, _selected)) {
                _selected = _selected->GetParent();
                _current_game_board->RestoreGameState(
                    _selected->game_state);
            }
            point_map[key]->Delete();
        }
        update_nodes();
    }

    return true;
}

void GraphNode::UpdateGameState(const GameState& state) {
    _selected->game_state = state;
}

void GraphNode::AddGameState(const GameState& state) {
    TestNodePtr attempt = TestNode::AddChild(_selected);
    if (attempt) {
        _selected = attempt;
        _selected->game_state = state;
        update_nodes();
    } else {
        _current_game_board->RestoreGameState(_selected->game_state);
    }
}


void GraphNode::StopMerge(const GameState& state) {
    _merging = false;
    _current_game_board->StopMerge();
    _last_game_board->StopMerge();
    TestNodePtr attempt = TestNode::Merge(_selected, _prev_selected);
    assert(attempt);
    attempt->game_state = state;
    _selected = attempt;
    _current_game_board->RestoreGameState(_selected->game_state);
    update_nodes();
}

void GraphNode::set_click(ClickType click)
{
    _click = click;
}

void GraphNode::StartMerge() {
    if (!TestNode::CanMerge(_selected, _prev_selected)) {
        return;
    }
    _merging = true;
    _current_game_board->StartMerge(_last_game_board);
    _last_game_board->StartMerge(_current_game_board);
}

void GraphNode::update_nodes() {
    boundLines->clear();

    auto bounds = this->getContentSize();
    int max_x = bounds.width - 1;
    int max_y = bounds.height - 1;

    boundLines->drawRect(Vec2(0, 0), Vec2(max_x, max_y), Color4F::MAGENTA);

    auto nodes = std::list<TestNodePtr>();

    root->branch_id_ = 0;
    nodes.push_back(root);
    int x = 0;
    point_map.clear();
    while (true) {
        if (nodes.empty()) {
            return;
        }
        auto next_nodes = std::list<TestNodePtr>();
        int y = 0;
        for (auto& parent : nodes) {
            uint64_t key = Point2Key(x, y);
            point_map[key] = parent;
            parent->branch_id_ = y;
            Vec2 pos(offset + x_space * x, offset + y_space * y);
            Color4F color;
            if (PtrEqual(parent, _selected)) {
                color = Color4F(1.0, 0.0, 1.0, 1.0);
            } else if (PtrEqual(parent, _prev_selected)) {
                color = Color4F(1.0, 1.0, 0.0, 1.0);
            } else {
                color = Color4F(0.0, 1.0, 1.0, 1.0);
            }
            boundLines->drawDot(pos, radius, color);
            if (x > 0) {
                auto ids = parent->GetParentBranchIDs();
                for (auto id : ids) {
                    Vec2 parent_pos(offset + x_space * (x - 1),
                        offset + y_space * (id));
                    boundLines->drawLine(pos, parent_pos,
                        Color4F(1.0, 1.0, 0.0, 1.0));
                }
            }
            for (auto& child : parent->GetChildren()) {
                // check if this is the second parent of a merge
                auto found_iter = std::find(next_nodes.begin(),
                    next_nodes.end(), child);
                if (found_iter == next_nodes.end()) {
                    next_nodes.push_back(child);
                }
            }
            y++;
        }
        x++;
        nodes = next_nodes;
    }
}
