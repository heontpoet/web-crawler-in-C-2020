/*
* Program made by : Ian Teh Wen Jing
* Created on : 30/3/2020
* Last modified on : 30/3/2020
*
* Simple web crawler using socket programming and libtidy to crawl through
* similar urls untill 100 unique urls are reached
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "client.h"

#include "tidy_library/include/tidy.h"
#include "tidy_library/include/tidybuffio.h"



int main(int argc, char** argv) {

  // Check if correct number of inputs are given
  if (argc < 2)
  {
    fprintf(stderr, "usage %s hostname\n", argv[0]);
    exit(0);
  }
  crawl(argv[1]);
  return 0;
}
