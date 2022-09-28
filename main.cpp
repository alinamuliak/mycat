// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <string>
#include "options_parser.h"

#define BUF_SIZE 10000


int open_files(char* filenames[], size_t array_length, std::vector<int>& descriptors, int *status) {
    int fd;

    for (size_t i = 1; i < array_length; ++i) {
        if (!strcmp(filenames[i], "-A") || !strcmp(filenames[i], "--A-flag")) {
            continue;
        }
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

int read_to_buffer(int file_descriptor, char buf[], size_t buffer_size, int *status) {
    int read_bytes = 0;
    while (read_bytes < buffer_size) {
        int read_now = read(file_descriptor, buf, buffer_size - read_bytes);
        if (read_now == 0) {
            break;
        }
        if (read_now == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                *status = errno;
                return -1;
            }
        } else {
            read_bytes += read_now;
        }
    }
    return read_bytes;
}

int write_to_stdout(size_t buf_size, char buf[], bool A_flag, int *status) {
    if (A_flag) {
        std::string formatted_buffer;
        char hex_code[5];
        for (size_t i = 0; i < buf_size; ++i) {
            if (!isprint(buf[i]) && !isspace(buf[i])) {
                sprintf(hex_code, "\\x%2X", (unsigned char) buf[i]);
                formatted_buffer.append(hex_code);
            } else {
                formatted_buffer.push_back(buf[i]);
            }
        }
        buf_size = formatted_buffer.size();
        buf = formatted_buffer.data();
    }

    size_t written_bytes = 0;
    while (written_bytes < buf_size) {
        int written_now = write(1, buf + written_bytes, buf_size - written_bytes);
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
    return 0;
}


int cat(const int argc, char* argv[], bool A_flag) {
    int status;
    int result;
    std::vector<int> descriptors;
    result = open_files(argv, argc, descriptors, &status);
    if (result != 0) {
        perror("");
        return status;
    }

    for (auto descr : descriptors) {
        char buf[BUF_SIZE + 1];
        int n;

        while ((n = read_to_buffer(descr, buf, BUF_SIZE, &status)) > 0) {
            if (n < 0) {
                perror("");
                return status;
            }
            if ((result = write_to_stdout(n, buf, A_flag, &status)) != 0) {
                perror("");
                return status;
            }
        }
    }
    return 0;
}


int main(int argc, char* argv[]) {
    command_line_options_t command_line_options{argc, argv};
    std::cout << "A flag value: " << command_line_options.get_A_flag() << std::endl;

    cat(argc, argv, command_line_options.get_A_flag());
    return 0;
}
