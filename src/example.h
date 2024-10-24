#ifndef EXAMPLE_EXAMPLE_H
#define EXAMPLE_EXAMPLE_H

#include <dbusx/bus.h>
#include <dbusx/interface.h>
#include <dbusx/error.h>
#include <dbusx/typedef.h>

#include <tl/expected.hpp>

#include <stdint.h>

class example : public dbusx::interface {
public:
    example(const dbusx::bus &conn);

    uint32_t string_length(const std::string &str);
    std::string echo(const std::string &str);

    tl::expected<std::string, dbusx::error> may_error(bool error);

    dbusx::object_path get_path();
    void no_return();
    tl::expected<void, dbusx::error> no_return_or_error(bool error);

    std::string multi_params(const std::string &str1, const std::string &str2);

    std::vector<std::string> array(const std::vector<std::string> &a);
    std::unordered_map<std::string, int32_t> dict(
        const std::unordered_map<std::string, int32_t> &d);

    std::tuple<std::string, int> struct_(const std::tuple<std::string, int> &s);

    void emitEmptySignal();

    std::string get_read_only_propery();
    tl::expected<std::string, dbusx::error> get_read_only_propery_e();

    std::string get_writable_propery();
    void set_writable_propery(const std::string &str);

    std::string interface_name() const override;
    dbusx::object_path path() const override;
    dbusx::vtable::vtable exported() const override;

private:
    dbusx::bus conn_;
    std::string writable_property_;
};

#endif // !EXAMPLE_EXAMPLE_H
