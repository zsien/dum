#ifndef DBUSX_INTERFACE_H
#define DBUSX_INTERFACE_H

#include "dbusx/typedef.h"

#include <string>

namespace dbusx {

namespace vtable {
struct vtable;
}

/*!
  @brief Base for all D-Bus interface
 */
class interface {
public:
    interface() = default;
    virtual ~interface() = default;
    virtual std::string interface_name() const = 0;
    virtual object_path path() const = 0;
    virtual vtable::vtable exported() const = 0;
};

} // namespace dbusx

#endif //! DBUSX_INTERFACE_H
