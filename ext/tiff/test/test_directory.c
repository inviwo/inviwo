/*
 * TIFF Library
 *
 * The purpose of this test suite is to test the correctness of TIFFWriteDirectory() when appending multiple directories
 * to an open file.
 *
 * Currently, there is an optimization where the TIFF data structure stores the offset of the last written directory in
 * order to avoid having to traverse the entire directory list each time a new one is added. The offset is not stored
 * in the file itself, only in the in-memory data structure. This means we still go through the entire list the first
 * time a directory is appended to a newly-opened file, and the shortcut is taken for subsequent directory writes.
 *
 * In order to test the correctness of the optimization, the `test_lastdir_offset` function writes 10 directories to two
 * different files. For the first file we use the optimization, by simply calling TIFFWriteDirectory() repeatedly on an
 * open TIFF handle. For the second file, we avoid the optimization by closing the file after each call to
 * TIFFWriteDirectory(), which means the next directory write will traverse the entire list.
 *
 * Finally, the two files are compared to check that the number of directories written is the same, and that their
 * offsets match. The test is then repeated using BigTIFF files.
 *
 */

#include "tif_config.h"

#include <stdio.h>
#include <stdbool.h>

#include "tiffio.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#define    SPP              3        /* Samples per pixel */
#define    N_DIRECTORIES    10       /* Number of directories to write */
const uint16_t width = 1;
const uint16_t length = 1;
const uint16_t bps = 8;
const uint16_t photometric = PHOTOMETRIC_RGB;
const uint16_t rows_per_strip = 1;
const uint16_t planarconfig = PLANARCONFIG_CONTIG;

int write_data_to_current_directory(TIFF *tif) {
    unsigned char buf[SPP] = {0, 127, 255};

    if (!tif) {
        fprintf(stderr, "Invalid TIFF handle.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width)) {
        fprintf(stderr, "Can't set ImageWidth tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, length)) {
        fprintf(stderr, "Can't set ImageLength tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps)) {
        fprintf(stderr, "Can't set BitsPerSample tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, SPP)) {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip)) {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PLANARCONFIG, planarconfig)) {
        fprintf(stderr, "Can't set PlanarConfiguration tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric)) {
        fprintf(stderr, "Can't set PhotometricInterpretation tag.\n");
        return 1;
    }

    /* Write dummy pixel data. */
    if (TIFFWriteScanline(tif, buf, 0, 0) == -1) {
        fprintf(stderr, "Can't write image data.\n");
        return 1;
    }

    return 0;
}

int write_directory_to_closed_file(const char *filename, bool is_big_tiff) {
    TIFF *tif;
    tif = TIFFOpen(filename, is_big_tiff ? "a8" : "a");
    if (!tif) {
        fprintf(stderr, "Can't create/open %s\n", filename);
        return 1;
    }

    if (write_data_to_current_directory(tif)) {
        fprintf(stderr, "Can't write data to current directory.\n");
        TIFFClose(tif);
        return 1;
    }

    if (!TIFFWriteDirectory(tif)) {
        fprintf(stderr, "TIFFWriteDirectory() failed.\n");
        TIFFClose(tif);
        return 1;
    }

    TIFFClose(tif);
    return 0;
}

int count_directories(const char *filename, int *count) {
    TIFF *tif;
    *count = 0;

    tif = TIFFOpen(filename, "r");
    if (!tif) {
        fprintf(stderr, "Can't read %s\n", filename);
        return 1;
    }

    do {
        (*count)++;
    } while (TIFFReadDirectory(tif));

    TIFFClose(tif);
    return 0;
}

/* Gets a list of the directory offsets in a file. Assumes the file has at least N_DIRECTORIES directories */
int get_dir_offsets(const char *filename, uint64_t *offsets) {
    TIFF *tif;
    int i;

    tif = TIFFOpen(filename, "r");
    if (!tif) {
        fprintf(stderr, "Can't read %s\n", filename);
        return 1;
    }

    for (i = 0; i < N_DIRECTORIES; i++) {
        offsets[i] = TIFFCurrentDirOffset(tif);
        if (!TIFFReadDirectory(tif) && i < (N_DIRECTORIES-1)) {
            fprintf(stderr, "Can't read %d.th directory from %s\n", i, filename);
            TIFFClose(tif);
            return 2;
        }
    }

    TIFFClose(tif);
    return 0;
}

/* Checks that rewriting a directory does not break the directory linked list
 *
 * This could happen because TIFFRewriteDirectory relies on the traversal of the directory linked list in order to
 * move the rewritten directory to the end of the file. This means the `lastdir_offset` optimization should be skipped,
 * otherwise the linked list will be broken at the point where it connected to the rewritten directory, resulting in the
 * loss of the directories that come after it.
*/
int test_rewrite_lastdir_offset(bool is_big_tiff) {
    const char *filename = "test_directory_rewrite.tif";
    int i, count;
    TIFF *tif;

/* Create a file and write N_DIRECTORIES (10) directories to it */
    tif = TIFFOpen(filename, is_big_tiff ? "w8" : "w");
    if (!tif) {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++) {
        if (write_data_to_current_directory(tif)) {
            fprintf(stderr, "Can't write data to current directory in %s\n", filename);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif)) {
            fprintf(stderr, "Can't write directory to %s\n", filename);
            goto failure;
        }
    }

    /* Without closing it, go to the fifth directory */
    TIFFSetDirectory(tif, 4);

    /* Rewrite the fifth directory by calling TIFFRewriteDirectory */
    if (write_data_to_current_directory(tif)) {
        fprintf(stderr, "Can't write data to fifth directory in %s\n", filename);
        goto failure;
    }
    if (!TIFFRewriteDirectory(tif)) {
        fprintf(stderr, "Can't rewrite fifth directory to %s\n", filename);
        goto failure;
    }

    TIFFClose(tif);
    tif = NULL;

    /* Check that the file has the expected number of directories*/
    if (count_directories(filename, &count)) {
        fprintf(stderr, "Error counting directories in file %s.\n", filename);
        goto failure;
    }
    if (count != N_DIRECTORIES) {
        fprintf(stderr, "Unexpected number of directories in %s. Expected %i, found %i.\n", filename,
                N_DIRECTORIES, count);
        goto failure;
    }

    unlink(filename);
    return 0;

    failure:
    if (tif) {
        TIFFClose(tif);
        tif = NULL;
    }
    unlink(filename);
    return 1;
}

/* Compares multi-directory files written with and without the lastdir optimization */
int test_lastdir_offset(bool is_big_tiff) {
    const char *filename_optimized = "test_directory_optimized.tif";
    const char *filename_non_optimized = "test_directory_non_optimized.tif";
    int i, count_optimized, count_non_optimized;
    uint64_t offsets_optimized[N_DIRECTORIES];
    uint64_t offsets_non_optimized[N_DIRECTORIES];
    TIFF *tif;

    /* First file: open it and add multiple directories. This uses the lastdir optimization. */
    tif = TIFFOpen(filename_optimized, is_big_tiff ? "w8" : "w");
    if (!tif) {
        fprintf(stderr, "Can't create %s\n", filename_optimized);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++) {
        if (write_data_to_current_directory(tif)) {
            fprintf(stderr, "Can't write data to current directory in %s\n", filename_optimized);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif)) {
            fprintf(stderr, "Can't write directory to %s\n", filename_optimized);
            goto failure;
        }
    }
    TIFFClose(tif);
    tif = NULL;

    /* Second file: close it after adding each directory. This avoids the lastdir optimization. */
    for (i = 0; i < N_DIRECTORIES; i++) {
        if (write_directory_to_closed_file(filename_non_optimized, is_big_tiff)) {
            fprintf(stderr, "Can't write directory to %s\n", filename_non_optimized);
            goto failure;
        }
    }

    /* Check that both files have the expected number of directories */
    if (count_directories(filename_optimized, &count_optimized)) {
        fprintf(stderr, "Error counting directories in file %s.\n", filename_optimized);
        goto failure;
    }
    if (count_optimized != N_DIRECTORIES) {
        fprintf(stderr, "Unexpected number of directories in %s. Expected %i, found %i.\n", filename_optimized,
                N_DIRECTORIES, count_optimized);
        goto failure;
    }
    if (count_directories(filename_non_optimized, &count_non_optimized)) {
        fprintf(stderr, "Error counting directories in file %s.\n", filename_non_optimized);
        goto failure;
    }
    if (count_non_optimized != N_DIRECTORIES) {
        fprintf(stderr, "Unexpected number of directories in %s. Expected %i, found %i.\n", filename_non_optimized,
                N_DIRECTORIES, count_non_optimized);
        goto failure;
    }

    /* Check that both files have the same directory offsets */
    if (get_dir_offsets(filename_optimized, offsets_optimized)) {
        fprintf(stderr, "Error reading directory offsets from %s.\n", filename_optimized);
        goto failure;
    }
    if (get_dir_offsets(filename_non_optimized, offsets_non_optimized)) {
        fprintf(stderr, "Error reading directory offsets from %s.\n", filename_non_optimized);
        goto failure;
    }
    for (i = 0; i < N_DIRECTORIES; i++) {
        if (offsets_optimized[i] != offsets_non_optimized[i]) {
            fprintf(stderr,
                    "Unexpected directory offset for directory %i, expected offset %"PRIu64" but got %"PRIu64".\n",
                    i,
                    offsets_non_optimized[i], offsets_optimized[i]);
            goto failure;
        }
    }

    unlink(filename_optimized);
    unlink(filename_non_optimized);

    return 0;

    failure:
    if (tif) {
        TIFFClose(tif);
        tif = NULL;
    }
    unlink(filename_optimized);
    unlink(filename_non_optimized);
    return 1;
}

int main() {
    if (test_lastdir_offset(false)) {
        fprintf(stderr, "Failed during non-BigTIFF WriteDirectory test.\n");
        return 1;
    }
    if (test_lastdir_offset(true)) {
        fprintf(stderr, "Failed during BigTIFF WriteDirectory test.\n");
        return 1;
    }


    if (test_rewrite_lastdir_offset(false)) {
        fprintf(stderr, "Failed during non-BigTIFF RewriteDirectory test.\n");
        return 1;
    }
    if (test_rewrite_lastdir_offset(true)) {
        fprintf(stderr, "Failed during BigTIFF RewriteDirectory test.\n");
        return 1;
    }
    return 0;
}
