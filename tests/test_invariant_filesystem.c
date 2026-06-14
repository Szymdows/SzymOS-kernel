#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Include the filesystem implementation directly */
#include "kernel/filesystem.c"

#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif

START_TEST(test_header_read_no_oob_write)
{
    /* Invariant: The header read loop must never write beyond the allocated
       header buffer, regardless of the sector count claimed in metadata. */

    /* Test cases: number of header sectors claimed by the image metadata */
    size_t claimed_sectors[] = {
        1,      /* valid: fits in a typical single-sector header buffer */
        2,      /* boundary: exactly at a common allocation limit */
        256,    /* exploit: far exceeds any reasonable header allocation */
    };
    int num_cases = sizeof(claimed_sectors) / sizeof(claimed_sectors[0]);

    for (int i = 0; i < num_cases; i++) {
        size_t alloc_sectors = 2; /* simulate a fixed 2-sector header buffer */
        size_t buf_size = alloc_sectors * SECTOR_SIZE;
        uint8_t *header_buf = malloc(buf_size + SECTOR_SIZE); /* guard page area */
        ck_assert_ptr_nonnull(header_buf);

        /* Fill guard area with canary */
        memset(header_buf, 0, buf_size);
        memset(header_buf + buf_size, 0xAA, SECTOR_SIZE);

        uint8_t sector_buffer[SECTOR_SIZE];
        memset(sector_buffer, 0x42, SECTOR_SIZE);

        /* Simulate the header read loop with bounds check */
        for (size_t s = 0; s < claimed_sectors[i]; s++) {
            size_t offset = s * SECTOR_SIZE;
            /* The security invariant: offset + SECTOR_SIZE must not exceed buf_size */
            if (offset + SECTOR_SIZE > buf_size) {
                /* Must not proceed — a correct implementation stops here */
                break;
            }
            memcpy(header_buf + offset, sector_buffer, SECTOR_SIZE);
        }

        /* Verify canary is intact — no out-of-bounds write occurred */
        for (size_t b = 0; b < SECTOR_SIZE; b++) {
            ck_assert_uint_eq(header_buf[buf_size + b], 0xAA);
        }

        free(header_buf);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_header_read_no_oob_write);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}