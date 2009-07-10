#include "common.hpp"
#include "divert.hpp"

#include <iostream>

const char *progname;
bool  verbose = false;

void usage();

int
main(int argc, char *argv[])
{
        int n;
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

        while ((ch = getopt(argc, argv, "p:b:e:t:n:v")) != -1){
                switch (ch){
                case 'p':
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
                case 'n':
                        n = atoi(optarg);
                        if (n == 0) {
                                type = full_cone;
                        } else if (n == 1) {
                                type = restricted_cone;
                        } else if (n == 2) {
                                type = port_restricted_cone;
                        }
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
                std::cerr << "please use -p option for divert socket"
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
                  << " [-b port] [-e port] [-t ttl] [-n 0 | 1 | 2] [-v] -p port\n"
                  << "    b: beginning port number, default value is 0\n"
                  << "    e: ending port number, default value is 65535\n"
                  << "    t: time to live of mapping, default value is 300\n"
                  << "    n: select filter type.\n"
                  << "       choose 0, 1 or 2. default value is 0.\n"
                  << "         0: filter like full cone NAT\n"
                  << "         1: filter like restricted cone NAT\n"
                  << "         2: filter like port-restricted cone NAT\n"
                  << "    v: toggle verbose mode\n"
                  << "    p: port number of divert socket"
                  << std::endl;
}
