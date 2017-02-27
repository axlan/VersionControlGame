#include "Util.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

Point2D Pixel2Point(float x, float y, float offset_x, float offset_y, float width, float height)
{
  int point_x = int((x - offset_x) / width + .5);
  int point_y = int((y - offset_y) / height + .5);
  return Point2D{ point_x, point_y };
}

uint64_t Point2Key(int32_t x, int32_t y)
{
  return (int64_t(x) << 32ull) + int64_t(y);
}

uint64_t Point2Key(Point2D val)
{
  return Point2Key(val.x, val.y);
}

cocos2d::Point GetWorldToNodeScale(const cocos2d::Node *node)
{
  return node->convertToNodeSpace(node->getPosition() + cocos2d::Point(1, 1));
}

bool IsAtribute(const cocos2d::TMXTiledMap* map, int gid, const char* attr)
{
  if (gid) {
    cocos2d::Value val = map->getPropertiesForGID(gid);
    if (!val.isNull()) {
      cocos2d::ValueMap properties = val.asValueMap();
      if (properties.count(attr) && properties[attr].asBool()) {
        return true;
      }
    }
  }
  return false;
}


template<typename Out>
static void split(const std::string &s, char delim, Out result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}
