#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define WRITE_N 1024
#define N 512

int data1[2 * WRITE_N * bsg_tiles_X * bsg_tiles_Y] __attribute__ ((section (".dram"))) = {0};

int main()
{
  bsg_set_tile_x_y();

  // create a 1024KB int array
  int start = __bsg_id * WRITE_N;
  for (int i = start; i < start + WRITE_N; i++)
  {
    data1[i] = 42;
  }

  // write the second half
  start += WRITE_N * bsg_tiles_X * bsg_tiles_Y;
  for (int i = start; i < start + WRITE_N; i++)
  {
    data1[i] = 43;
  }

  bsg_fence();

  int buf[N];

  // read first 256KB, which hopefully wont have much in L2
  start = __bsg_id * N;

  for (int iter = 1; iter < 4; iter++) {

    bsg_cuda_print_stat_start(iter);
    bsg_fence();

    int buf_offset = 0;
    for (int idx = start; idx < start + N; idx++) {
      buf[buf_offset] = data1[idx];
      buf_offset++;
    }

    bsg_cuda_print_stat_end(iter);
    bsg_fence();

    buf_offset = 0;

    int acc = 0;
    for (int idx = start; idx < start + N; idx++) {
      acc += buf[idx];
    }
    if (acc != 42 * N) bsg_fail();
  }

  bsg_finish();
  bsg_wait_while(1);
}
