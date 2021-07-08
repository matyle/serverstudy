#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

//暂时用不上namespace
namespace Matt{
class noncopyable
{
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

}
#endif 