#include "GraphNode.h"
#include "GameBoard.h"

//////////////////GRAPH CODE///////////////////////////////////////////

#include <list>
#include <vector>
#include <memory>
#include <assert.h>

#include "Util.h"

class TestNode;

typedef std::shared_ptr<TestNode> TestNodePtr;
typedef std::list<TestNodePtr> TestNodePtrList;

class TestNode
{
public:
  static TestNodePtr MakeRoot(int max_branches, int max_t)
  {
    return TestNodePtr(new TestNode(max_branches, max_t));
  }

  static TestNodePtr AddChild(TestNodePtr smart_this)
  {
	  auto& branches = *smart_this->branches_;
	  int new_t = smart_this->t_ + 1;
	  if (new_t >= smart_this->max_t_ || (branches.size() > new_t && branches[new_t] == smart_this->max_branches_)) {
		  return nullptr;
	  }
	  TestNodePtr child(new TestNode(smart_this));
	  smart_this->children_.push_back(child);
	  return child;
  }

  static TestNodePtr Merge(TestNodePtr a, TestNodePtr b)
  {
	  if (!a || !b || a->t_ != b->t_) {
		  return nullptr;
	  }
	  auto& branches = *a->branches_;
	  int new_t = a->t_ + 1;
	  if (new_t >= a->max_t_ || (branches.size() > new_t && branches[new_t] == a->max_branches_)) {
		  return nullptr;
	  }
	  TestNodePtr child(new TestNode(a));
	  child->parent2_ = b;
	  a->children_.push_back(child);
	  b->children_.push_back(child);
	  return child;
  }


  const TestNodePtrList& GetChildren()
  {
    return children_;
  }

  bool Delete()
  {
    if (children_.size() > 0 || !parent1_) {
      return false;
    }
    (*branches_)[t_]--;
	parent1_->RemoveChild(this);
	if (parent2_) {
		parent2_->RemoveChild(this);
	}
  }

  int branch_id_ = -1;

  std::vector<int> GetParentBranchIDs()
  {
	std::vector<int> parent_ids;
	parent_ids.push_back(parent1_->branch_id_);
	if (parent2_) {
		parent_ids.push_back(parent2_->branch_id_);
	}
    return parent_ids;
  }

  TestNodePtr GetParent()
  {
	  return parent1_;
  }

  GameState game_state;



protected:

  void RemoveChild(const TestNode* target)
  {
	  for (auto iterator = this->children_.begin(); iterator != this->children_.end(); ++iterator) {
		  TestNodePtr& child = *iterator;
		  if (child.get() == target) {
			  this->children_.remove(child);
			  return;
		  }
	  }
	  assert(false);
  }

  TestNode(int max_branches, int max_t) : max_branches_(max_branches), max_t_(max_t), parent1_(nullptr), parent2_(nullptr), t_(0)
  {
    branches_ = std::make_shared<std::vector<int>>();
    branches_->push_back(1);
  }
  TestNode(TestNodePtr parent) : max_branches_(parent->max_branches_), max_t_(parent->max_t_), parent1_(parent), parent2_(nullptr), t_(parent->t_ + 1), branches_(parent->branches_)
  {
    auto& branches = *branches_;
    if (branches.size() <= t_) {
      branches.push_back(1);
    }
    else {
      branches[t_]++;
    }
  }

  const int max_branches_;
  const int max_t_;
  std::shared_ptr<std::vector<int>> branches_;
  TestNodePtr parent1_;
  TestNodePtr parent2_;
  std::list<TestNodePtr> children_;
  int t_;
};



/////////////////////////////////////////////////////////////////////



USING_NS_CC;

void GraphNode::set_game_boards(GameBoard* current, GameBoard* last)
{
	_current_game_board = current;
	_last_game_board = last;
}



// on "init" you need to initialize your instance
bool GraphNode::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Node::init())
	{
		return false;
	}
	_current_game_board = nullptr;
	_last_game_board = nullptr;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
  boundLines = DrawNode::create();
  this->addChild(boundLines);


  root = TestNode::MakeRoot(7, 25);

  _prev_selected = nullptr;
  _selected = root;


  auto listener = EventListenerMouse::create();
  listener->onMouseDown = CC_CALLBACK_1(GraphNode::on_mouse_down, this);

  listener->onMouseMove = [](cocos2d::Event* event) {
    // Cast Event to EventMouse for position details like above
    //cocos2d::log("Mouse moved event");
  };

  listener->onMouseScroll = [](cocos2d::Event* event) {
    //cocos2d::log("Mouse wheel scrolled");
  };

  listener->onMouseUp = [](cocos2d::Event* event) {
    //cocos2d::log("Mouse button released");
  };

  _eventDispatcher->addEventListenerWithFixedPriority(listener, 1);

	return true;
}

const int x_space = 30;
const int y_space = 30;
const int radius = 10;
const int offset = 10;

static Point2D Pixel2Point(int x, int y)
{
  return Pixel2Point(x, y, offset, offset, x_space, y_space);
}

template <class T>
static bool PtrEqual(T a, T b)
{
	return a.get() == b.get();
}

void GraphNode::on_mouse_down(cocos2d::Event* event) {
  try {
    EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
    mouseEvent->getMouseButton();
    std::stringstream message;

    Point touchLocation = mouseEvent->getLocationInView();
    //touchLocation = CCDirector::sharedDirector()->convertToGL(touchLocation);
    touchLocation = convertToNodeSpace(touchLocation);
    //CCLOG(" TouchLocation X=%f TouchLocation Y=%f", touchLocation.x, touchLocation.y);

    Point2D point = Pixel2Point(touchLocation.x, touchLocation.y);

    //CCLOG(" point X=%d point Y=%d", point.x, point.y);

    uint64_t key = Point2Key(point);

    if (point_map.count(key) > 0) {
      if (mouseEvent->getMouseButton() == MOUSE_BUTTON_LEFT) {
		  if (PtrEqual(point_map[key], _selected)) {
			  return;
		  }
		  _last_game_board->RestoreGameState(_selected->game_state);
		  _prev_selected = _selected;
		  _selected = point_map[key];
		  _current_game_board->RestoreGameState(_selected->game_state);
      }
      else {
		TestNodePtr target = point_map[key];
		if (PtrEqual(target, _prev_selected)) {
			_prev_selected = nullptr;
		}
		if (PtrEqual(target, _selected)) {
		  _selected = _selected->GetParent();
		  _current_game_board->RestoreGameState(_selected->game_state);
		}
        point_map[key]->Delete();
      }
	  update_nodes();
    }

    

    /*
    message << "Mouse event: Button: " << mouseEvent->getMouseButton() << "pressed at point (" <<
      mouseEvent->getLocation().x << "," << mouseEvent->getLocation().y << ")";
    MessageBox(message.str().c_str(), "Mouse Event Details");
    */
  }
  catch (std::bad_cast& e) {
    log(e.what());
    // Not sure what kind of event you passed us cocos, but it was the wrong one
    return;
  }
}

void GraphNode::UpdateGameState(const GameState& state)
{
	_selected->game_state = state;
}

void GraphNode::AddGameState(const GameState& state)
{
	TestNodePtr attempt = TestNode::AddChild(_selected);
	if (attempt) {
		_selected = attempt;
		_selected->game_state = state;
		update_nodes();
	}
	else {
		_current_game_board->RestoreGameState(_selected->game_state);
	}
}

void GraphNode::Merge() {
	TestNodePtr attempt = TestNode::Merge(_selected, _prev_selected);
	if (attempt) {
		attempt->game_state = _selected->game_state;
		_selected = attempt;
		_current_game_board->RestoreGameState(_selected->game_state);
		update_nodes();
	}
}

void GraphNode::update_nodes() {

  boundLines->clear();

  auto bounds = this->getContentSize();
  int max_x = bounds.width - 1;
  int max_y = bounds.height - 1;

  boundLines->drawLine(Vec2(0, 0), Vec2(0, max_y), Color4F(1.0, 0.0, 1.0, 1.0));
  boundLines->drawLine(Vec2(0, 0), Vec2(max_x, 0), Color4F(1.0, 0.0, 1.0, 1.0));
  boundLines->drawLine(Vec2(max_x, max_y), Vec2(0, max_y), Color4F(1.0, 0.0, 1.0, 1.0));
  boundLines->drawLine(Vec2(max_x, max_y), Vec2(max_x, 0), Color4F(1.0, 0.0, 1.0, 1.0));

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
    for each (auto& parent in nodes)
    {
      uint64_t key = Point2Key(x, y);
      point_map[key] = parent;
      parent->branch_id_ = y;
      Vec2 pos(offset + x_space * x, offset + y_space * y);
	  Color4F color;
	  if (PtrEqual(parent, _selected)) {
		  color = Color4F(1.0, 0.0, 1.0, 1.0);
	  }
	  else if (PtrEqual(parent, _prev_selected)) {
		  color = Color4F(1.0, 1.0, 0.0, 1.0);
	  }
	  else {
		  color = Color4F(0.0, 1.0, 1.0, 1.0);
	  }
      boundLines->drawDot(pos, radius, color);
      if (x > 0) {
		auto ids = parent->GetParentBranchIDs();
		for (auto id : ids) {
			Vec2 parent_pos(offset + x_space * (x - 1), offset + y_space * (id));
			boundLines->drawLine(pos, parent_pos, Color4F(1.0, 1.0, 0.0, 1.0));
		}
      }
      for each (auto& child in parent->GetChildren())
      {
		//check if this is the second parent of a merge
		auto found_iter = std::find(next_nodes.begin(), next_nodes.end(), child);
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
