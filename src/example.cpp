#include "example.h"

#include <dbusx/method.h>
#include <dbusx/property.h>
#include <dbusx/signal.h>
#include <dbusx/error.h>

#include <iostream>

example::example(const dbusx::bus &conn)
    : conn_(conn) {
}

uint32_t example::string_length(const std::string &str) {
    return str.length();
}

std::string example::echo(const std::string &str) {
    return str;
}

tl::expected<std::string, dbusx::error> example::may_error(bool error) {
    if (error) {
        return tl::make_unexpected(
            dbusx::error("org.freedesktop.DBus.Error.FileNotFound", "error message"));
    }

    return {"no error ~"};
}

dbusx::object_path example::get_path() {
    return dbusx::object_path("/example");
}

void example::no_return() {
}

tl::expected<void, dbusx::error> example::no_return_or_error(bool error) {
    if (error) {
        return tl::make_unexpected(
            dbusx::error("org.freedesktop.DBus.Error.FileNotFound", "error message"));
    }

    return {};
}

std::string example::multi_params([[maybe_unused]] const std::string &str1,
                                  const std::string &str2) {
    return str2;
}

std::vector<std::string> example::array(const std::vector<std::string> &a) {
    return a;
}

std::tuple<std::string, int> example::struct_(const std::tuple<std::string, int> &s) {
    std::cout << std::get<0>(s) << std::endl;
    std::cout << std::get<1>(s) << std::endl;
    return s;
}

void example::emitEmptySignal() {
    // conn_.
    dbusx::message::new_signal(conn_, path(), interface_name(), "EmptySignal").send();
}

std::unordered_map<std::string, int32_t> example::dict(
    const std::unordered_map<std::string, int32_t> &d) {
    return d;
}

std::string example::get_read_only_propery() {
    return "read only !!!";
}

tl::expected<std::string, dbusx::error> example::get_read_only_propery_e() {
    return tl::make_unexpected(
        dbusx::error("org.freedesktop.DBus.Error.FileNotFound", "error message"));
}

std::string example::get_writable_propery() {
    return writable_property_;
}

void example::set_writable_propery(const std::string &str) {
    writable_property_ = str;
    conn_.emit_property_changed(this, "WritableProperty");
}

std::string example::interface_name() const {
    return "org.deepin.UpdateManager1";
}

dbusx::object_path example::path() const {
    return dbusx::object_path("/org/deepin/UpdateManager1");
}

dbusx::vtable::vtable example::exported() const {
    return {
        .methods =
            {
                {"StringLength", dbusx::method<&example::string_length>::get_vtable()},
                {"Echo", dbusx::method<&example::echo>::get_vtable()},
                {"MayError", dbusx::method<&example::may_error>::get_vtable()},
                {"GetPath", dbusx::method<&example::get_path>::get_vtable()},
                {"NoReturn", dbusx::method<&example::no_return>::get_vtable()},
                {"NoReturnOrError", dbusx::method<&example::no_return_or_error>::get_vtable()},
                {"MultiParams", dbusx::method<&example::multi_params>::get_vtable()},
                {"Array", dbusx::method<&example::array>::get_vtable()},
                {"Dict", dbusx::method<&example::dict>::get_vtable()},
                {"Struct", dbusx::method<&example::struct_>::get_vtable()},
                {"EmitEmptySignal", dbusx::method<&example::emitEmptySignal>::get_vtable()},
            },
        .properties =
            {
                {"ReadOnlyProperty",
                 dbusx::property<&example::get_read_only_propery>::get_vtable()},
                {"ReadOnlyPropertyE",
                 dbusx::property<&example::get_read_only_propery_e>::get_vtable()},
                {"WritableProperty",
                 dbusx::property<&example::get_writable_propery,
                                 &example::set_writable_propery>::get_vtable()},
            },
        .signals =
            {
                {"EmptySignal", dbusx::signal<>::get_vtable()},
            },
    };
}
