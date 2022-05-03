#ifndef __UNCOPYABLE_H__
#define __UNCOPYABLE_H__

class Uncopyable {
 public:
  Uncopyable() = default;
  ~Uncopyable() = default;

 private:
  Uncopyable(const Uncopyable&) = delete;
  Uncopyable& operator=(const Uncopyable&) = delete;
};

#endif  // __UNCOPYABLE_H__