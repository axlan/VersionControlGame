#include "GraphNode.h"


//////////////////GRAPH CODE///////////////////////////////////////////

#include <list>
#include <vector>
#include <memory>
#include <assert.h>

class TestNode;

typedef std::shared_ptr<TestNode> TestNodePtr;
typedef std::list<TestNodePtr> TestNodePtrList;

class TestNode
{
public:
  static TestNodePtr MakeRoot(int max_branches)
  {
    return TestNodePtr(new TestNode(max_branches));
  }

  const TestNodePtrList& GetChildren()
  {
    return children_;
  }

  TestNodePtr AddChild()
  {
    auto& branches = *branches_;
    if (branches.size() > t_ + 1 && branches[t_ + 1] == max_branches_) {
      return nullptr;
    }
    TestNodePtr child(new TestNode(this));
    children_.push_back(child);
    return child;
  }

  bool Delete()
  {
    if (children_.size() > 0 || !parent_) {
      return false;
    }
    (*branches_)[t_]--;
    for (auto iterator = parent_->children_.begin(); iterator != parent_->children_.end(); ++iterator) {
      TestNodePtr& child = *iterator;
      if (child.get() == this) {
        parent_->children_.remove(child);
        return true;
      }
    }
    assert(false);
  }

  int branch_id_ = -1;

  int GetParentBranchID()
  {
    return parent_->branch_id_;
  }

protected:
  TestNode(int max_branches) : max_branches_(max_branches), parent_(nullptr), t_(0)
  {
    branches_ = std::make_shared<std::vector<int>>();
    branches_->push_back(1);
  }
  TestNode(TestNode* parent) : max_branches_(parent->max_branches_), parent_(parent), t_(parent->t_ + 1), branches_(parent->branches_)
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
  std::shared_ptr<std::vector<int>> branches_;
  TestNode* parent_;
  std::list<TestNodePtr> children_;
  int t_;
};



/////////////////////////////////////////////////////////////////////





USING_NS_CC;


struct Point2D
{
  int32_t x;
  int32_t y;
};

uint64_t Point2Key(int32_t x, int32_t y)
{
  return (int64_t(x) << 32ull) + int64_t(y);
}


uint64_t Point2Key(Point2D val)
{
  return Point2Key(val.x, val.y);
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


	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
  boundLines = DrawNode::create();
  this->addChild(boundLines);


  root = TestNode::MakeRoot(7);


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

  need_update = true;

  this->scheduleUpdate();

	return true;
}


const int x_space = 30;
const int y_space = 30;
const int radius = 10;
const int offset = 10;


Point2D Pixel2Point(int x, int y)
{
  int point_x = int(float(x - offset) / x_space + .5);
  int point_y = int(float(y - offset) / y_space + .5);
  return Point2D{ point_x, point_y };
}


void GraphNode::on_mouse_down(cocos2d::Event* event) {
  try {
    EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
    mouseEvent->getMouseButton();
    std::stringstream message;

    CCPoint touchLocation = mouseEvent->getLocationInView();
    //touchLocation = CCDirector::sharedDirector()->convertToGL(touchLocation);
    touchLocation = convertToNodeSpace(touchLocation);
    CCLOG(" TouchLocation X=%f TouchLocation Y=%f", touchLocation.x, touchLocation.y);

    Point2D point = Pixel2Point(touchLocation.x, touchLocation.y);

    CCLOG(" point X=%d point Y=%d", point.x, point.y);

    uint64_t key = Point2Key(point);

    if (point_map.count(key) > 0) {
      if (mouseEvent->getMouseButton() == MOUSE_BUTTON_LEFT) {
        point_map[key]->AddChild();
      }
      else {
        point_map[key]->Delete();
      }
      need_update = true;
    }

    

    /*
    message << "Mouse event: Button: " << mouseEvent->getMouseButton() << "pressed at point (" <<
      mouseEvent->getLocation().x << "," << mouseEvent->getLocation().y << ")";
    MessageBox(message.str().c_str(), "Mouse Event Details");
    */
  }
  catch (std::bad_cast& e) {
    // Not sure what kind of event you passed us cocos, but it was the wrong one
    return;
  }
}



void GraphNode::update(float delta) {

  if (!need_update) {
    return;
  }
  boundLines->clear();

  need_update = false;

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
      boundLines->drawDot(pos, radius, Color4F(1.0, 0.0, 1.0, 1.0));
      if (x > 0) {
        Vec2 parent_pos(offset + x_space * (x - 1), offset + y_space * (parent->GetParentBranchID()));
        boundLines->drawLine(pos, parent_pos, Color4F(1.0, 1.0, 0.0, 1.0));
      }
      for each (auto& child in parent->GetChildren())
      {
        next_nodes.push_back(child);
      }
      y++;
    }
    x++;
    nodes = next_nodes;
  }









}
