// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <string>
#include "options_parser.h"

#define BUF_SIZE 80000


int open_files(char* filenames[], int array_length, std::vector<int>& descriptors, int *status) {
    int fd;

    for (int i = 1; i < array_length; ++i) {
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

int write_to_stdout(int buf_size, char buf[], int *status) {
    int written_bytes = 0;
    char formatted_buffer[BUF_SIZE * 4];
    int current_size = 0;
    for (int i = 0; i < buf_size; ++i) {
        std::cout << buf[i] << std::endl;
        if ((isprint(buf[i]) == 0) && (isspace(buf[i]) == 0)) {
            char hex_code[4];
            sprintf(hex_code, "\\x%2X", (unsigned char)buf[i]);
            for (auto ch: hex_code) {
                if (ch == ' ') {
                    continue;
                }
                formatted_buffer[current_size] = ch;
                current_size += 1;
            }
        } else {
            formatted_buffer[current_size] = buf[i];
            current_size += 1;
        }
    }
    while (written_bytes < current_size) {
        int written_now = write(1, formatted_buffer + written_bytes, current_size - written_bytes);
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


int cat(const int argc, char* argv[]) {
    // todo: add -A flag

    int status;
    int result;
    std::vector<int> descriptors;
    result = open_files(argv, argc, descriptors, &status);
    if (result != 0) {
        perror("");
        return status;
    }

    for (auto descr : descriptors) {
        char buf[BUF_SIZE];

        int read_bytes = read_to_buffer(descr, buf, BUF_SIZE, &status);
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
