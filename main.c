/*
 * main.c
 *
 *  Created on: May 15, 2011
 *      Author: John Heaton
 */

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define IMG3_MAGIC 0x496D6733

typedef struct {
    uint32_t magic;
    uint32_t full_size;
    uint32_t image_size;
    uint32_t signature_offset;
    uint32_t image_identifier;
} img3_header;

typedef struct {
    uint32_t section_type;
    uint32_t full_size;
    uint32_t data_size;
} img3_section;

void print_flip_endian_32(uint32_t *value) {
    char id[5], reverse[5];
    id[4] = 0;
    reverse[4] = 0;

    memcpy(reverse, value, 4);

    id[0] = reverse[3];
    id[1] = reverse[2];
    id[2] = reverse[1];
    id[3] = reverse[0];

    printf("%s", id);
}

void print_img3_header(img3_header *header) {
    printf("\n=========\n");
    printf("Header\n");
    printf("=========\n");
    printf("Full size: 0x%x(%d\n", header->full_size, header->full_size);
    printf("Image size: 0x%x(%d)\n", header->image_size, header->image_size);
    printf("SHSH offset: 0x%x\n", header->signature_offset);
    printf("Image id: ");
    print_flip_endian_32(&header->image_identifier);
    printf("\n\n");
}

void print_img3_section(img3_section *section, unsigned int offset) {
    printf("Section: ");
    print_flip_endian_32(&section->section_type);
    printf("\n");
    printf("Offset: 0x%x(%d)\n", offset, offset);
    printf("Full size: 0x%x(%d)\n", section->full_size, section->full_size);
    printf("Data size: 0x%x(%d)\n\n", section->data_size, section->data_size);
}

void die_with_err(const char *desc, int code) {
    if(desc != NULL) {
        printf("Error %d: %s\n", code, desc);
    } else {
        printf("Error %d\n", code);
    }

    exit(code);
}

void usage(const char *progname, const char *arg_fmt, char die, int exit_code) {
    printf("Usage: %s %s\n", progname, arg_fmt);

    if(die) {
        die_with_err(NULL, exit_code);
    }
}

int main(int argc, const char **argv) {
    int file;
    void *mappedimg;
    struct stat st;

    if(argc < 2) {
        usage("img3info", "<file>", 1, -1);
    }

    if(stat(argv[1], &st) != 0) {
        die_with_err("Couldn't find file", -1);
    }

    file = open(argv[1], O_RDONLY);
    if(file == -1) {
        die_with_err("Couldn't open file for reading", -1);
    }

    mappedimg = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, file, 0);
    if(mappedimg == MAP_FAILED) {
        close(file);
        die_with_err("Couldn't map file", -1);
    }

    if(*((uint32_t *)mappedimg) != IMG3_MAGIC) {
        munmap(mappedimg, st.st_size);
        close(file);
        die_with_err("File isn't a valid IMG3 container", -1);
    }

    print_img3_header((img3_header *)mappedimg);

    printf("\n=========\n");
    printf("Data\n");
    printf("=========\n");
    
    int offset = sizeof(img3_header);
    while(offset < ((img3_header *)mappedimg)->image_size) {
        img3_section *section = (img3_section *)(mappedimg+offset);

        print_img3_section(section, offset - sizeof(img3_header));

        offset += section->full_size;
    }

    munmap(mappedimg, st.st_size);
    close(file);

    return 0;
}
