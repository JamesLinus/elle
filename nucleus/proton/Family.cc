#include <nucleus/proton/Family.hh>
#include <nucleus/Exception.hh>

#include <iostream>
#include <stdexcept>

namespace nucleus
{
  namespace proton
  {

    std::ostream&
    operator <<(std::ostream& stream,
                Family const family)
    {
      switch (family)
        {
        case FamilyUnknown:
          {
            stream << "unknown";
            break;
          }
        case FamilyContentHashBlock:
          {
            stream << "content hash block";
            break;
          }
        case FamilyPublicKeyBlock:
          {
            stream << "public key block";
            break;
          }
        case FamilyOwnerKeyBlock:
          {
            stream << "owner key block";
            break;
          }
        case FamilyImprintBlock:
          {
            stream << "imprint block";
            break;
          }
        default:
          {
            throw Exception("unknown family: '%s'", family);
          }
        }

      return (stream);
    }

  }
}
