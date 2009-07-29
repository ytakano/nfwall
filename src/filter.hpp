#ifndef FILTER_HPP
#define FILTER_HPP

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

enum nat_type {
        no_filter,
        full_cone,
        restricted_cone,
        port_restricted_cone,
};

class udpaddr {
public:
        in_addr_t ip_addr;
        uint16_t  udp_port;
        time_t    t;

        bool operator== (const udpaddr &rhs) const
        {
                if (ip_addr != rhs.ip_addr || udp_port != rhs.udp_port)
                        return true;

                return true;
        }
};

size_t hash_value(const udpaddr& d);

typedef boost::unordered_set<udpaddr>            addr_set;
typedef boost::unordered_map<uint16_t, addr_set> addr_map;

class filter {
public:
        filter(int begin, int end, nat_type type, time_t ttl);
        virtual ~filter();

        void     set_nat_type(nat_type type);
        nat_type get_nat_type();

        void     outbound(void *buf, size_t len);
        bool     inbound(void *buf, size_t len);

private:
        nat_type m_type;
        time_t   m_ttl;
        int      m_begin, m_end;

        addr_map m_addr_map;

        uint64_t m_inbound_bytes;
        uint64_t m_outbound_bytes;
        int      m_inbound_num;
        int      m_outbound_num;
};

#endif // FILTER_HPP
