#ifndef DBUSX_SIGNAL_H
#define DBUSX_SIGNAL_H

#include "type.h"
#include "vtable.h"

namespace dbusx {

template <typename... T>
struct signal {
    static vtable::signal get_vtable() {
        return {
            .signatures = types<T...>::signature_str.c_str(),
            .names = {},
            .flags = 0,
        };
    }
};

} // namespace dbusx

#endif // !DBUSX_SIGNAL_H
