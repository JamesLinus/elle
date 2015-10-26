#ifndef INFINIT_REACTOR_STORAGE_HH
# define INFINIT_REACTOR_STORAGE_HH

# include <mutex>
# include <unordered_map>

# include <boost/signals2.hpp>

# include <reactor/fwd.hh>

namespace reactor
{
  template <typename T>
  class LocalStorage
  {
  public:
    typedef LocalStorage<T> Self;
    LocalStorage();
    ~LocalStorage();
    operator T&();
    T& Get(T const& def);
    T& Get();

  private:
    void _Clean(Thread* t);
    typedef std::unordered_map<void*, T> Content;
    Content _content;
    typedef std::unordered_map<void*, boost::signals2::connection> Links;
    Links _links;
    std::mutex _mutex;
  };
}

# include <reactor/storage.hxx>

#endif
