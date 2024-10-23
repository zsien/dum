#include <systemd/sd-daemon.h>

#include <uvw.hpp>

#include <spdlog/spdlog.h>

#include <unordered_map>
#include <string>
// #include <any>

// class amap {
// public:
//     class iterator {
//         friend class amap_base;

//     private:
//         std::any base_;
//     };
// };

// class amap_base {
// public:
//     virtual ~amap_base() = default;

//     virtual amap::iterator begin() = 0;

// protected:
//     static amap::iterator iterator_from_base(std::any base) {
//         amap::iterator it;
//         it.base_ = base;

//         return it;
//     }
// };

// template <typename K, typename V>
// class amap_t : public amap_base {
// public:
//     explicit amap_t(const std::unordered_map<K, V> &data) : data_(data) {}

//     amap::iterator begin() override {
//         return iterator_from_base(data_.begin());
//     }

// private:
//     std::unordered_map<K, V> data_;
// };

static std::unordered_map<std::string, int> get_fds() {
    std::unordered_map<std::string, int> fds;
    char **names;
    int count = sd_listen_fds_with_names(true, &names);
    for (int i = 0; i < count; i++) {
        fds.emplace(names[i], SD_LISTEN_FDS_START + i);
        free(names[i]);
    }
    free(names);

    return fds;
}

int main() {
    auto fds = get_fds();
    if (!fds.contains("upgrade-stdout")) {
        return 1;
    }

    auto loop = uvw::loop::get_default();
    std::shared_ptr<uvw::tcp_handle> tcp = loop->resource<uvw::tcp_handle>();
    tcp->on<uvw::listen_event>([](const uvw::listen_event &, uvw::tcp_handle &srv) {
        std::shared_ptr<uvw::tcp_handle> client = srv.parent().resource<uvw::tcp_handle>();
        spdlog::info("new client connect: {}", client->peer().ip);

        client->on<uvw::close_event>([ptr = srv.shared_from_this()] (const uvw::close_event &, uvw::tcp_handle &) {
            spdlog::info("client disconnect: {}", ptr->peer().ip);
            ptr->close();
        });
        client->on<uvw::end_event>([](const uvw::end_event &, uvw::tcp_handle &client) { client.close(); });

        srv.accept(*client);
        client->read();
    });
    tcp->open(fds["upgrade-stdout"]);

    loop->run();
}
