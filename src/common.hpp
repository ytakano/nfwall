#ifndef COMMON_HPP
#define COMMON_HPP

extern const char *progname;
extern bool verbose;

#define PERROR(MSG)                                                     \
        do {                                                            \
                char str[1024];                                         \
                snprintf(str, sizeof(str), "%s: %s:%d[%s]: %s",        \
                         progname, __FILE__, __LINE__, __func__, MSG);  \
                perror(str);                                            \
        } while(0);

#endif // COMMON_HPP
