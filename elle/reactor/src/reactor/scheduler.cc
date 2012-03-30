#include <boost/foreach.hpp>

#include <reactor/debug.hh>
#include <reactor/scheduler.hh>
#include <reactor/thread.hh>

namespace reactor
{
  /*-------------.
  | Construction |
  `-------------*/

  Scheduler::Scheduler()
    : _current(0)
    , _starting()
    , _starting_mtx()
    , _running()
    , _frozen()
    , _io_service()
    , _io_service_work(_io_service)
    , _manager()
  {}

  /*----.
  | Run |
  `----*/

  void
  Scheduler::run()
  {
    while (true)
    {
      // Could avoid locking if no jobs are pending with a boolean.
      {
        boost::unique_lock<boost::mutex> lock(_starting_mtx);
        _running.insert(_starting.begin(), _starting.end());
        _starting.clear();
      }
      Threads running(_running);
      INFINIT_REACTOR_DEBUG("Scheduler: new round with "
                            << running.size() << " jobs");
      BOOST_FOREACH (Thread* t, running)
      {
        INFINIT_REACTOR_DEBUG("Scheduler: schedule " << *t);
        _current = t;
        try
        {
          t->_step();
        }
        catch (const std::runtime_error& err)
        {
          std::cerr << "thread " << t->name() << ": "
                    << err.what() << std::endl;
          std::abort();
        }
        catch (...)
        {
          std::cerr << "thread " << t->name() << ": "
                    << "unknown error" << std::endl;
          std::abort();
        }
        _current = 0;
        if (t->state() == Thread::state::done)
        {
          INFINIT_REACTOR_DEBUG("Scheduler: cleanup " << *t);
          _running.erase(t);
        }
      }
      INFINIT_REACTOR_DEBUG("Scheduler: run asio callbacks");
      _io_service.reset();
      _io_service.poll();
      if (_running.empty() && _starting.empty())
        if (_frozen.empty())
          break;
        else
          while (_running.empty() && _starting.empty())
          {
            INFINIT_REACTOR_DEBUG("Scheduler: nothing to do, "
                                  "polling asio in a blocking fashion");
            _io_service.reset();
            boost::system::error_code err;
            std::size_t run = _io_service.run_one(err);
            if (err)
            {
              std::cerr << "fatal ASIO error: " << err << std::endl;
              std::abort();
            }
            else if (run == 0)
            {
              std::cerr << "ASIO service is dead." << std::endl;
              std::abort();
            }
          }
    }
    INFINIT_REACTOR_DEBUG("Scheduler: done");
    assert(_frozen.empty());
  }

  /*-------------------.
  | Threads management |
  `-------------------*/

  Thread*
  Scheduler::current() const
  {
    return _current;
  }

  void
  Scheduler::_freeze(Thread& thread)
  {
    assert(thread.state() == Thread::state::running);
    assert(_running.find(&thread) != _running.end());
    _running.erase(&thread);
    _frozen.insert(&thread);
  }

  static void nothing()
  {}

  void
  Scheduler::_thread_register(Thread& thread)
  {
    // FIXME: be thread safe only if needed
    {
      boost::unique_lock<boost::mutex> lock(_starting_mtx);
      _starting.insert(&thread);
      // Wake the scheduler.
      _io_service.post(nothing);
    }
  }

  void
  Scheduler::_unfreeze(Thread& thread)
  {
    assert(thread.state() == Thread::state::frozen);
    _frozen.erase(&thread);
    _running.insert(&thread);
  }

  /*-----.
  | Asio |
  `-----*/

  boost::asio::io_service&
  Scheduler::io_service()
  {
    return _io_service;
  }
}
