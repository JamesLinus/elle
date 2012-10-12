#ifndef DIARY_MEMOIRS_HH
# define DIARY_MEMOIRS_HH

# include <boost/noncopyable.hpp>

# include <elle/types.hh>
# include <elle/concept/Fileable.hh>
# include <elle/io/Dumpable.hh>
# include <elle/Buffer.hh>


namespace satellite
{

  ///
  /// this class represents the diary in its archived form.
  ///
  class Memoirs
    : public elle::radix::Object
    , public elle::io::Dumpable
    , public elle::serialize::Serializable<Memoirs>
    , public elle::concept::Fileable<>
    , private boost::noncopyable
  {
  public:
    Memoirs();
    elle::Status        Dump(const elle::Natural32 = 0) const;

    // XXX
    elle::Buffer         archive;
    size_t                        offset;
  };

}

#include <satellites/diary/Memoirs.hxx>

#endif
