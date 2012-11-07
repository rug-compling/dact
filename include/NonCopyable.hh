#ifndef DACT_NONCOPYABLE_HPP
#define DACT_NONCOPYABLE_HPP

/**
 * Base class for objects without copy constructor and assignment.
 */
class NonCopyable {
  protected:
    NonCopyable() {}
    ~NonCopyable() {}
  private:
    NonCopyable(NonCopyable const &);
    NonCopyable &operator=(NonCopyable const &);
};

#endif  // DACT_NONCOPYABLE_HPP
