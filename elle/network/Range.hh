//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       network
//
// file          /home/mycure/infinit/elle/network/Range.hh
//
// created       julien quintard   [fri mar 26 13:07:25 2010]
// updated       julien quintard   [sat mar 27 05:59:26 2010]
//

#ifndef ELLE_NETWORK_RANGE_HH
#define ELLE_NETWORK_RANGE_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/core/Natural.hh>
#include <elle/core/Character.hh>

namespace elle
{
  namespace network
  {

//
// ---------- structures ------------------------------------------------------
//

    ///
    /// this structure defines the range capacity.
    ///
    template <const Natural32 S>
    struct Capacity
    {
      static const Natural32	Size = S;
    };

    ///
    /// this structure calculates the dependency' ranges.
    ///
    template <const Character*... D>
    struct Dependency
    {
    };

    ///
    /// this structure defines a range of tags.
    ///
    template <const Character* C>
    struct Range
    {
    };

  }
}

//
// ---------- templates -------------------------------------------------------
//

#include <elle/network/Range.hxx>

#endif
