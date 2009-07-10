#include "divert.hpp"

#include <iostream>

#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
open_divert(uint16_t port)
{
        struct sockaddr_in bind_port;
        int fd;

        fd = socket(PF_INET, SOCK_RAW, IPPROTO_DIVERT);
        if (fd < 0) {
                PERROR("socket");
                return -1;
        }

        bzero(&bind_port, sizeof(bind_port));

        bind_port.sin_family = PF_INET;
        bind_port.sin_port   = htons(port);

        if (bind(fd, (struct sockaddr*)&bind_port, sizeof(bind_port)) < 0) {
                close(fd);
                PERROR("bind");
                return -1;
        }

        return fd;
}

void
print_packet(uint8_t *buf, ssize_t size)
{
        ip *hdr;

        hdr = (ip*)buf;


        // print IP
        std::cout << "   IP:" << std::endl;
        std::cout << "       src: " << inet_ntoa(hdr->ip_src) << std::endl;
        std::cout << "       dst: " << inet_ntoa(hdr->ip_dst) << std::endl;

        if (hdr->ip_p == IPPROTO_UDP) {
                std::cout << "       protocol: UDP" << std::endl;
        } else if (hdr->ip_p == IPPROTO_TCP) {
                std::cout << "       protocol: TCP" << std::endl;
        } else {
                std::cout << "       protocol: " << (int)hdr->ip_p
                          << std::endl;
        }

        if (hdr->ip_p != IPPROTO_UDP)
                return;


        // print UDP
        udphdr *udp;

        udp = (udphdr*)(buf + hdr->ip_hl * 4);

        std::cout << "   UDP:\n"
                  << "       src: " << ntohs(udp->uh_sport) << "\n"
                  << "       dst: " << ntohs(udp->uh_dport) << std::endl;
}

void
read_loop(int fd, filter &fil)
{
        sockaddr_in  sin;
        socklen_t    sin_len;
        ssize_t      size;
        uint8_t      buf[1024 * 1024];

        for (;;) {
                sin_len = sizeof(sin);
                size = recvfrom(fd, buf, sizeof(buf), 0,
                                (sockaddr*)&sin, &sin_len);

                if (size < 0) {
                        PERROR("recvfrom");
                        exit(-1);
                }

                if (verbose) {
                        std::cout << "receive " << size << "[bytes]:"
                                  << std::endl;

                        if (sin.sin_addr.s_addr == INADDR_ANY) {
                                std::cout << "   diection: outbound"
                                          << std::endl;
                        } else {
                                std::cout << "   diection: inbound"
                                          << std::endl;

                                std::cout << "   received IF address: "
                                          << inet_ntoa(sin.sin_addr)
                                          << std::endl;
                        }

                        print_packet(buf, size);
                }

                if (sin.sin_addr.s_addr == INADDR_ANY) {
                        fil.outbound(buf, size);
                        sendto(fd, buf, size, 0,
                               (struct sockaddr*)&sin, sin_len);
                } else {
                        if (fil.inbound(buf, size)) {
                                sendto(fd, buf, size, 0,
                                       (struct sockaddr*)&sin, sin_len);

                                if (verbose) {
                                        std::cout << "   handle: forwarded"
                                                  << std::endl;
                                }
                        } else {
                                if (verbose) {
                                        std::cout << "   handle: filtered"
                                                  << std::endl;
                                }
                        }
                }
        }
}
