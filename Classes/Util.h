#pragma once

#include <stdint.h>
#include "cocos2d.h"

struct Point2D
{
  int32_t x;
  int32_t y;
  Point2D() :x(0), y(0)
  {
  }
  Point2D(int32_t x, int32_t y):x(x),y(y)
  {
  }

  Point2D operator+(const Point2D& rhs) const
  {
    return Point2D(x + rhs.x, y + rhs.y); // return the result by value (uses move constructor)
  }
  Point2D operator-(const Point2D& rhs) const
  {
    return Point2D(x - rhs.x, y - rhs.y); // return the result by value (uses move constructor)
  }
  bool operator==(const Point2D& other) const
  {
	  return x == other.x && y == other.y;
  }
  bool operator!=(const Point2D& other) const
  {
	  return !(*this == other);
  }
};

Point2D Pixel2Point(float x, float y, float offset_x, float offset_y, float width, float height);

uint64_t Point2Key(int32_t x, int32_t y);

uint64_t Point2Key(Point2D val);

cocos2d::Point GetWorldToNodeScale(const cocos2d::Node *node);

bool IsAtribute(const cocos2d::TMXTiledMap* map, int gid, const char* attr);

std::vector<std::string> split(const std::string &s, char delim);