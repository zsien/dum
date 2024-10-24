#include "dbusx/bus.h"

#include "bus_private.h"
#include "dbusx/interface.h"

#include <systemd/sd-bus.h>

#include <stdexcept>
#include <memory>

using namespace dbusx;

bus::bus(bus_type type)
    : d_ptr_(std::make_unique<bus_private>()) {
    decltype(&sd_bus_open) f;

    switch (type) {
    case bus_type::USER:
        f = &sd_bus_open_user;
        break;
    case bus_type::SYSTEM:
        f = &sd_bus_open_system;
        break;
    case bus_type::STARTER:
        f = &sd_bus_open;
        break;
    }

    int ret = f(&d_ptr_->conn_);
    if (ret < 0) {
        throw std::runtime_error(std::string("Failed to connect to dbus: ") + strerror(-ret));
    }
}

bus::~bus() {
    sd_bus_flush_close_unref(d_ptr_->conn_);
}

bool bus::request_name(const std::string &name) {
    return sd_bus_request_name(d_ptr_->conn_, name.c_str(), 0) >= 0;
}

bool bus::release_name(const std::string &name) {
    return sd_bus_release_name(d_ptr_->conn_, name.c_str()) >= 0;
}

// TODO: 优化
bool bus::export_interface(interface *obj) {
    auto path = obj->path();
    auto iface = obj->interface_name();
    auto &exported_interfaces = d_ptr_->exported_[path];

    if (exported_interfaces.contains(iface)) {
        // this interface exported
        return false;
    }

    auto exported = obj->exported();

    auto d = std::make_unique<data>();

    d->ud.reserve(exported.methods.size() +  // methods
                  exported.properties.size() // properties
    );

    d->vtable.reserve(1 +                          // start
                      exported.methods.size() +    // methods
                      exported.properties.size() + // properties
                      exported.signals.size() +    // signals
                      1                            // end
    );

    d->vtable.push_back(SD_BUS_VTABLE_START(0));

    size_t current_offset = 0;
    for (const auto &i : exported.methods) {
        d->ud.emplace_back(userdata{
            .i = obj,
            .caller = {
                .invoke = i.second.invoker,
            },
        });

        d->vtable.push_back(SD_BUS_METHOD_WITH_OFFSET(i.first,
                                                      i.second.in_signatures,
                                                      i.second.out_signatures,
                                                      &bus_private::on_method_call,
                                                      current_offset,
                                                      SD_BUS_VTABLE_UNPRIVILEGED));

        current_offset += sizeof(userdata);
    }

    for (const auto &i : exported.properties) {
        d->ud.emplace_back(userdata{
            .i = obj,
            .caller = {
                .property = {
                    .get = i.second.getter,
                    .set = i.second.setter,
                },
            },
        });

        if (i.second.setter == nullptr) {
            d->vtable.push_back(SD_BUS_PROPERTY(i.first,
                                                i.second.signature,
                                                &bus_private::on_property_get,
                                                current_offset,
                                                SD_BUS_VTABLE_PROPERTY_CONST));
        } else {
            d->vtable.push_back(SD_BUS_WRITABLE_PROPERTY(i.first,
                                                         i.second.signature,
                                                         &bus_private::on_property_get,
                                                         &bus_private::on_property_set,
                                                         current_offset,
                                                         SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE));
        }

        current_offset += sizeof(userdata);
    }

    for (const auto &i : exported.signals) {
        d->vtable.push_back(SD_BUS_SIGNAL(i.first,
                                          i.second.signatures,
                                          0));
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    d->vtable.push_back(SD_BUS_VTABLE_END);
#pragma GCC diagnostic pop

    int ret = sd_bus_add_object_vtable(d_ptr_->conn_,
                                       nullptr,
                                       std::string(path).c_str(),
                                       iface.c_str(),
                                       d->vtable.data(),
                                       d->ud.data());

    exported_interfaces.emplace(iface, std::move(d));

    // TODO: 错误处理
    return ret < 0;
}

// TODO: 优化、错误处理
void bus::start() {
    int ret;
    for (;;) {
        ret = sd_bus_process(d_ptr_->conn_, nullptr);
        if (ret < 0) {
            fprintf(stderr, "Failed to process bus: %s\n", strerror(-ret));
            break;
        }
        if (ret > 0) {
            continue;
        }

        ret = sd_bus_wait(d_ptr_->conn_, UINT64_MAX);
        if (ret < 0) {
            fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-ret));
            break;
        }
    }
}

int bus::get_fd() const {
    return sd_bus_get_fd(d_ptr_->conn_);
}

int bus::process() const {
    return sd_bus_process(d_ptr_->conn_, nullptr);
}

bool bus::emit_property_changed(const interface *obj, const std::string &name) {
    return sd_bus_emit_properties_changed(d_ptr_->conn_, std::string(obj->path()).c_str(), obj->interface_name().c_str(), name.c_str(), nullptr) >= 0;
}
