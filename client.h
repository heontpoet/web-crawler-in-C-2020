#ifndef CLIENT_H_
#define CLIENT_H_

/*
* Header file for client.c
*
*/

#include "tidy_library/include/tidy.h"
#include "tidy_library/include/tidybuffio.h"

// Function to travers the TidyDoc(html file)
void dumpNode(TidyDoc doc, TidyNode tnod, char* host, char* path, char urls_array[100][1000]);

// Main function to do web crawling
void crawl(char* given_url);

#endif
