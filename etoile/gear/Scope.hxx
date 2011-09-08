//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/gear/Scope.hxx
//
// created       julien quintard   [fri jul 29 12:54:19 2011]
// updated       julien quintard   [wed sep  7 15:14:49 2011]
//

#ifndef ETOILE_GEAR_SCOPE_HXX
#define ETOILE_GEAR_SCOPE_HXX

namespace etoile
{
  namespace gear
  {

//
// ---------- methods ---------------------------------------------------------
//

    ///
    /// this method returns the context associated with the scope
    /// by casting it with the given type.
    ///
    template <typename T>
    elle::Status	Scope::Use(T*&				context)
    {
      enter();

      // first, if the scope's context is null, allocate one.
      if (this->context == NULL)
	{
	  // allocate a context according to the nature.
	  this->context = new T;
	}

      // return the context by dynamically casting it.
      if ((context = dynamic_cast<T*>(this->context)) == NULL)
	escape("invalid context nature");

      leave();
    }

  }
}

#endif
