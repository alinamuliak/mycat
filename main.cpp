// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <string>
#include "options_parser.h"

int cat(const int argc, char* argv[]) {
    // todo: add -A flag

    int fd;
    int n;
    std::vector<int> descriptors;

    // open files
    for (int i = 1; i < argc; ++i) {
        while ((fd = open(argv[i], O_RDONLY)) != 0) {
            if (errno == EINTR) {
                continue;
            } else {
                // *status = errno;
                return -1;
            }
        }
        descriptors.push_back(fd);
    }


    // read file by file
    for (auto descr : descriptors) {
        const size_t megabyte = 1000000;
        char buf[megabyte];

        // check for errors
        while ((n = read(descr, buf, sizeof(buf))) != 0) {
            if (n == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    // *status = errno;
                    return -1;
                }
            }

            // write + check for errors
            size_t written_bytes = 0;
            while (written_bytes < n) {
                size_t written_now = write(1, buf + written_bytes, n - written_bytes);
                if (written_now == -1) {
                    if (errno == EINTR) {
                        continue;
                    } else {
                        // *status = errno;
                        return -1;
                    }
                } else {
                    written_bytes += written_now;
                }
            }
        }
    }
    return 0;
}


int main(int argc, char* argv[]) {
    command_line_options_t command_line_options{argc, argv};
    std::cout << "A flag value: " << command_line_options.get_A_flag() << std::endl;

    cat(argc, argv);
    return 0;
}
