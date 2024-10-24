#ifndef DBUSX_BUS_H
#define DBUSX_BUS_H

#include <memory>

namespace dbusx {

class bus_private;
struct data;

class interface;
class message;

enum class bus_type { USER, SYSTEM, STARTER };

/*!
  D-Bus connection
 */
class bus {
    friend class bus_private;
    friend class message;

public:
    bus(bus_type type);
    ~bus();

    /*!
      @brief Request a well-known service name on the bus
      
      @param name the service name
      @return request result
     */
    bool request_name(const std::string &name);

    /*!
      @brief Release a well-known service name on the bus
      
      @param name the service name
      @return release result
     */
    bool release_name(const std::string &name);

    /*!
      @brief Export the interface object on the bus

      @param path the path of the interface
      @param iface the interface name
      @param obj the interface object
     */
    bool export_interface(interface *obj);

    /*!
      @brief Start the event loop
     */
    void start();

    int get_fd() const;
    int process() const;

    bool emit_property_changed(const interface *obj, const std::string &name);
    
    bool emit_signal(const interface *obj, const std::string &name, const message &msg);

private:
    std::shared_ptr<bus_private> d_ptr_;
};

} // namespace dbusx

#endif // !DBUSX_BUS_H
