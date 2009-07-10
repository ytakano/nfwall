#include "filter.hpp"

#include <arpa/inet.h>

size_t
hash_value(const udpaddr& d)
{
        size_t h = 0;
	boost::hash_combine(h, d.ip_addr);
	boost::hash_combine(h, d.udp_port);
	return h;
}

filter::filter(int begin, int end, nat_type type, time_t ttl)
{
        m_begin = begin;
        m_end   = end;
        m_type  = type;
        m_ttl   = ttl;
}

filter::~filter()
{

}

void
filter::set_nat_type(nat_type type)
{

}

nat_type
filter::get_nat_type()
{
        return m_type;
}

void
filter::outbound(void *buf, size_t len)
{
        m_outbound_bytes += len;
        m_outbound_num++;

        if (m_type == no_filter)
                return;


        udphdr  *udp;
        ip      *hdr;
        udpaddr  dst;
        uint16_t src_port;

        hdr = (ip*)buf;

        if (hdr->ip_p != IPPROTO_UDP)
                return;


        udp = (udphdr*)((char*)hdr + hdr->ip_hl * 4);
        src_port = ntohs(udp->uh_sport);

        if (src_port < m_begin || src_port > m_end)
                return;

        switch(m_type) {
        case full_cone:
                dst.ip_addr  = INADDR_ANY;
                dst.udp_port = 0;
                break;
        case restricted_cone:
                dst.ip_addr  = ntohl(hdr->ip_dst.s_addr);
                dst.udp_port = 0;
                break;
        case port_restricted_cone:
                dst.ip_addr  = ntohl(hdr->ip_dst.s_addr);
                dst.udp_port = ntohs(udp->uh_dport);
                break;
        default:
                ;
        }

        dst.t = time(NULL);

        m_addr_map[src_port].insert(dst);
}

bool
filter::inbound(void *buf, size_t len)
{
        m_inbound_bytes += len;
        m_inbound_num++;

        if (m_type == no_filter)
                return true;

        udphdr  *udp;
        ip      *hdr;
        udpaddr  src;
        uint16_t dst_port;
        time_t   t;
        addr_map::iterator it_map;
        addr_set::iterator it_set;

        hdr = (ip*)buf;

        if (hdr->ip_p != IPPROTO_UDP)
                return true;


        udp = (udphdr*)((char*)hdr + hdr->ip_hl * 4);
        dst_port = ntohs(udp->uh_dport);

        if (dst_port < m_begin || dst_port > m_end)
                return true;

        it_map = m_addr_map.find(dst_port);
        if (it_map == m_addr_map.end())
                return false;


        switch (m_type) {
        case full_cone:
                src.ip_addr  = INADDR_ANY;
                src.udp_port = 0;
                break;
        case restricted_cone:
                src.ip_addr  = ntohl(hdr->ip_src.s_addr);
                src.udp_port = 0;
                break;
        case port_restricted_cone:
                src.ip_addr  = ntohl(hdr->ip_src.s_addr);
                src.udp_port = ntohs(udp->uh_sport);
                break;
        default:
                ;
        }


        t = time(NULL);

        it_set = it_map->second.find(src);
        if (it_set == it_map->second.end()) {
                return false;
        } else if (t - it_set->t > m_ttl) {
                it_map->second.erase(it_set);
                if (it_map->second.size() == 0)
                        m_addr_map.erase(it_map);
                return false;
        }

        src.t = t;

        it_map->second.insert(src);

        return true;
}
