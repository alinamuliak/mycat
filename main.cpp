// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <string>
#include "options_parser.h"


int open_files(char* filenames[], int array_length, std::vector<int>& descriptors, int *status) {
    int fd;

    for (int i = 1; i < array_length; ++i) {
        while ((fd = open(filenames[i], O_RDONLY)) < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                 *status = errno;
                 return -1;
            }
        }
        descriptors.push_back(fd);
    }
    return 0;
}

int read_to_buffer(int file_descriptor, char buf[], int buffer_size, int *status) {
    int read_bytes = 0;
    int n;
    while ((n = read(file_descriptor, buf, buffer_size)) != 0) {
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                 *status = errno;
                return -1;
            }
        }
        read_bytes += n;
    }
    return read_bytes;
}

int write_to_stdout(int n, char buf[], int *status) {
    int written_bytes = 0;
    while (written_bytes < n) {
        int written_now = write(1, buf + written_bytes, n - written_bytes);
        if (written_now == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                *status = errno;
                return -1;
            }
        } else {
            written_bytes += written_now;
        }
    }
    // todo: should i chack for errno here too?
    write(1, "\n", 1);
    return 0;
}


int cat(const int argc, char* argv[]) {
    // todo: add -A flag

    int status;
    int result;
    std::vector<int> descriptors;
    // todo: check if there are flags in argv

    result = open_files(argv, argc, descriptors, &status);
    if (result != 0) {
        perror("");
        return status;
    }

    for (auto descr : descriptors) {
        const size_t megabyte = 80000;     // todo: change variable name
        char buf[megabyte];

        int read_bytes = read_to_buffer(descr, buf, megabyte, &status);
        if (read_bytes < 0) {
            perror("");
            return status;
        }

        if ((result = write_to_stdout(read_bytes, buf, &status)) != 0) {
            perror("");
            return status;
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
