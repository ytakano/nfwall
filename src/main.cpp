#include "common.hpp"
#include "divert.hpp"

#include <iostream>

const char *progname;
bool  verbose = false;

void usage();

int
main(int argc, char *argv[])
{
        int begin, end;
        int port;
        nat_type     type;
        time_t       ttl;


        int          ch;
        extern char *optarg;
        extern int   optind;

        progname = argv[0];

        begin = 0;
        end   = 65535;
        ttl   = 300;
        type  = full_cone;
        port  = 0;

        while ((ch = getopt(argc, argv, "d:b:e:t:frpv")) != -1){
                switch (ch){
                case 'd':
                        port = atoi(optarg);
                        break;
                case 'b':
                        begin = atoi(optarg);
                        break;
                case 'e':
                        end = atoi(optarg);
                        break;
                case 't':
                        ttl = atoi(optarg);
                        break;
                case 'f':
                        type = full_cone;
                        break;
                case 'r':
                        type = restricted_cone;
                        break;
                case 'p':
                        type = port_restricted_cone;
                        break;
                case 'v':
                        verbose = true;
                        break;
                default:
                        usage();
                        return -1;
                }
        }
        argc -= optind;
        argv += optind;


        if (port == 0) {
                std::cerr << "please use -d option for divert socket"
                          << std::endl;
                usage();
                return -1;
        }


        filter f(begin, end, type, ttl);
        int fd;

        fd = open_divert(port);
        if (fd < 0) {
                return -1;
        }

        read_loop(fd, f);

        return 0;
}

void
usage()
{
        std::cout << "usage:\n"
                  << "  "
                  << progname
                  << " [-b port] [-e port] [-t ttl] [-f | -r | -p] -d port\n"
                  << "    b: beginning port number, default value is 0\n"
                  << "    e: ending port number, default value is 65535\n"
                  << "    t: time to live of mapping, default value is 300\n"
                  << "    f: set full cone NAT\n"
                  << "    r: set restricted cone NAT\n"
                  << "    p: set port-restricted cone NAT\n"
                  << "    d: port number for divert socket"
                  << std::endl;
}
