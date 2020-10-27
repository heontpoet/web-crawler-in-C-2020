
/* Object file containing functions used in my crawler.c
 *
 * Code taken and modified from client.c program given in workshop 4
 * Code modified from example of htmltidy that was given
 */

#include <ctype.h>
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include <regex.h>
#include "tidy_library/include/tidy.h"
#include "tidy_library/include/tidybuffio.h"



#define NO_PATH ""
#define PATH_LENGTH 500
#define URL_LENGTH 1000
#define NUM_OF_UNIQUE_URLS 100
#define BUFFER_LEN 100000
#define HTTP_HEADERS 80

void dumpNode(TidyDoc doc, TidyNode tnod, char* host, char* path, char urls_array[100][1000])
{
  TidyNode child;
  int index;

  regex_t regex;
  int reti;

  // Create a copy of the url given
  char* first_component =  malloc(sizeof(char) * (strlen(host) + 1) );
  strcpy(first_component, host);

  char copy_of_path[URL_LENGTH];

  strcpy(copy_of_path, path);

  // Split the path into the first components and the last one
  char* last_part_of_path = strrchr(copy_of_path, '/');
  if (last_part_of_path != NULL) {
    *last_part_of_path = '\0';
  }

  int k = 0;
  // Find the host of the current url
  char* checking_host = strchr(first_component, '.');
  if (checking_host != NULL) {
    *checking_host++ = '\0';
    for (k=0; checking_host[k]!='\0' ; k++) {
      checking_host[k] = tolower(checking_host[k]);
    }
  }


  reti = regcomp(&regex, "http://", REG_EXTENDED);

  if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    exit(1);
  }

  // Loop through each tidy node to find the href, compare them into the urls_array
  // and if they are new and unique, add them into the array for future parsing
  for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
    TidyAttr hrefAttr = tidyAttrGetById(child, TidyAttr_HREF);
    if(hrefAttr && tidyAttrValue(hrefAttr) != NULL) {
      int k = 0;
      reti = regexec(&regex, tidyAttrValue(hrefAttr), 0 , NULL, 0);

      // Check if href starts with http
      if (reti) {
        char combinedURL[URL_LENGTH*3];
        if (strlen(tidyAttrValue(hrefAttr)) < URL_LENGTH)
        {
          if (tidyAttrValue(hrefAttr)[0] != '/') {
            sprintf(combinedURL, "http://%s.%s/%s/%s",first_component, checking_host, copy_of_path, tidyAttrValue(hrefAttr));
          } else if ( tidyAttrValue(hrefAttr)[0] == '/' && tidyAttrValue(hrefAttr)[1] == '/') {
            sprintf(combinedURL, "http:%s", tidyAttrValue(hrefAttr));
          } else if ( tidyAttrValue(hrefAttr)[0] == '/') {
            sprintf(combinedURL, "http://%s.%s%s",first_component, checking_host,tidyAttrValue(hrefAttr));
          } else if ( tidyAttrValue(hrefAttr)[0] == ' ') {
            int start_index = 0, end_index = strlen(tidyAttrValue(hrefAttr)), i;
            while(tidyAttrValue(hrefAttr)[index] == ' ') {
              start_index++;
            }
            char href[500];
            for(i =  start_index; isspace(tidyAttrValue(hrefAttr)[i]) == 0; i++) {
              href[i] = tidyAttrValue(hrefAttr)[i];
            }
            href[i++] = '\0';
            sprintf(combinedURL, "http://%s.%s/%s/%s",first_component, checking_host, copy_of_path, href);
          }

          int insertion_flag = 1;
          int char_index;

          for (index = 0; index < NUM_OF_UNIQUE_URLS; index++)
          {
            if (strncmp(combinedURL, urls_array[index], strlen(combinedURL)) == 0)
            {
              insertion_flag = 0;
            }
          }

          for (char_index  = 0; tidyAttrValue(hrefAttr)[char_index] != '\0'; char_index++) {
            if (tidyAttrValue(hrefAttr)[char_index] == '?' ||
                (strstr(tidyAttrValue(hrefAttr), "/..") != NULL) ||
                (strstr(tidyAttrValue(hrefAttr), "./") != NULL) ||
                (strstr(tidyAttrValue(hrefAttr), "/.") != NULL) ||
                tidyAttrValue(hrefAttr)[char_index] == '#') {
              insertion_flag = 0;
            }
          }

          if (insertion_flag == 1)
          {
            for (index = 0; index < NUM_OF_UNIQUE_URLS; index++)
            {
              if ((urls_array[index] ==  NULL || urls_array[index][0] != 'h') )
              {
                strncpy(urls_array[index], combinedURL, strlen(combinedURL));
                break;
              }
            }
          }
        }
      }
      else
      {
        if (strlen(tidyAttrValue(hrefAttr)) < URL_LENGTH) {
          int char_index, insertion_flag = 1;

          char newHost[URL_LENGTH], copy_of_original_host[URL_LENGTH];
          sscanf(tidyAttrValue(hrefAttr), "http://%99[^\n]",newHost);
          sprintf(copy_of_original_host, "%s/", host);

          char* path = strchr(newHost, '/');
          if (path != NULL) {
            *path++ = '\0';
          }

          if (path != NULL && strlen(path) == 0) {
            insertion_flag = 0;
          }

          if (path != NULL && strcmp(path, "") != 0) {
            for (char_index  = 0; path[char_index] != '\0'; char_index++) {
              if (path[char_index] == '?' ||
                  (strstr(path, "/..") != NULL) ||
                  (strstr(path, "./") != NULL) ||
                  (strstr(path, "/.") != NULL) ||
                  path[char_index] == '#') {
                insertion_flag = 0;
              }
            }
          }
          char* host_check = strchr(newHost, '.');
          if (host_check != NULL) {
            *host_check++ = '\0';
          }


          if (strcmp(host_check, checking_host) != 0) {
            insertion_flag = 0;
          }

          int index;
          for (index = 0; index < NUM_OF_UNIQUE_URLS; index++)
          {

            if (strncmp(tidyAttrValue(hrefAttr), urls_array[index], strlen(tidyAttrValue(hrefAttr))) == 0)
            {
              insertion_flag = 0;
              break;
            }
          }

          if (insertion_flag == 1)
          {
            for (index = 0; index < NUM_OF_UNIQUE_URLS; index++)
            {
              if ((urls_array[index] ==  NULL || urls_array[index][0] != 'h') )
              {
                strncpy(urls_array[index], tidyAttrValue(hrefAttr), strlen(tidyAttrValue(hrefAttr)));
                break;
              }
            }
          }
        }
      }
    }
    free(first_component);
    dumpNode(doc, child, host, path, urls_array); /* recursive */
  }
}


// Main function to crawl the URLs and parse html in the URls
void crawl(char* given_url)
{
    // Initialise the static array
    int num_parsed_urls = 0;;
    char unique_urls[NUM_OF_UNIQUE_URLS][URL_LENGTH];
    int current_url_index = 0;

    strncpy(unique_urls[0], given_url, strlen(given_url));
    num_parsed_urls++;



    // Main loop that runs until 100 unique urls are parsed
    while (num_parsed_urls < NUM_OF_UNIQUE_URLS)
    {
      int sockfd, portno;
      struct sockaddr_in serv_addr;
      struct hostent * server;

      char buffer[BUFFER_LEN];

      TidyDoc tdoc;
      TidyBuffer docbuf = {0};
      TidyBuffer tidy_errbuf = {0};

      tdoc = tidyCreate();
      tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */
      tidyOptSetInt(tdoc, TidyWrapLen, 4096);
      tidySetErrorBuffer(tdoc, &tidy_errbuf);
      tidyBufInit(&docbuf);





      /* Initialising variables
       */
      portno = 80;
      char host[URL_LENGTH];
      strcpy(host,unique_urls[current_url_index]);
      char header[BUFFER_LEN];
      char* path;


      /* Getting the host and path from the given url
       */
      printf("\n the host is %s \n", host);
      sscanf(host, "http://%s", host);

      path = strchr(host, '/');
      if (path != NULL) {
        *path++ = '\0';
      } else {
        path = NO_PATH;
      }

      /* Translate host name into peer's IP address ;
       * This is name translation service by the operating system
       */
      server = gethostbyname(host);

      if (server == NULL)
      {
          fprintf(stderr, "ERROR, no such host\n");
          exit(0);
      }


      /* Building data structures for socket */

      bzero((char *)&serv_addr, sizeof(serv_addr));

      serv_addr.sin_family = AF_INET;

      bcopy(server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);

      serv_addr.sin_port = htons(portno);

      /* Create TCP socket -- active open
      * Preliminary steps: Setup: creation of active open socket
      */

      sockfd = socket(AF_INET, SOCK_STREAM, 0);

      if (sockfd < 0)
      {
          perror("ERROR opening socket");
          exit(0);
      }

      if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      {
          perror("ERROR connecting");
          exit(0);
      }

      /* Creating the HTTP GET request to send to the server */

      sprintf(header, "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent:iteh\r\nConnection: close\r\n\r\n", path,host);

      /* Send the HTTP GET request to the server */
      send(sockfd,header,strlen(header),0);

      /* Use the recv function to accept the data sent by the site into the
       * buffer made and note down the total number of bytes sent by the server
       */
      int byte_count = 0;
      int total = 0;
      while ( (byte_count = recv(sockfd, &buffer[total], sizeof(buffer)-total,MSG_WAITALL)) > 0)
      {
        total += byte_count;
      }

      /* Split the received file into the HTTP Response header http http fields
       * and the message body
       */
      char *body = strstr(buffer, "\r\n\r\n");
      if (body != NULL)
      {
          *body++ = '\0';
      }

      char copy_body[strlen(buffer)];
      strncpy(copy_body, buffer, strlen(buffer));
      copy_body[strlen(buffer)] = '\0';

      char* http_fields[HTTP_HEADERS];

      // Split the HTTP Response header into individual fields
      http_fields[0] = strtok(copy_body, "\r\n");
      int i = 0, j = 0;
      while (http_fields[i] != NULL)
      {
        i++;
        http_fields[i] = strtok(NULL, "\r\n");
      }


      regex_t regex1, regex2, regex3;
      int reti1, reti2, reti3;
      char* supposed_total_bytes_sent_string;
      int supposed_total_bytes_sent_int = 0;

      // Regex expression used to find the HTTP status code
      reti1 = regcomp(&regex1, "HTTP/1.1 200 OK", 0);
      if (reti1) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
      }

      int status_flag = 0;
      for (j=0; j<=i-1; j++)
      {
        reti1 = regexec(&regex1, http_fields[j], 0 , NULL, 0);
        if (!reti1 )
        {
          status_flag = 1;
        }
      }

      if (status_flag == 1) {
        printf("\n Status code is 200");
      } else {
        printf("\n Status code is not 200 \n");
      }

      // Regex expression used to find the content length of the reponse
      reti2 = regcomp(&regex2, "Content-Length: [1-9]+", REG_EXTENDED);
      if (reti2) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
      }


      for (j=0; j<=i-1; j++)
      {
        reti2 = regexec(&regex2, http_fields[j], 0 , NULL, 0);
        if (!reti2 )
        {
          supposed_total_bytes_sent_string = malloc(sizeof(char) * strlen(http_fields[j])+1);
          sscanf(supposed_total_bytes_sent_string, "Content-Length: %s", http_fields[j]);
        }
      }

      // Check if page is truncated if content length field is present
      if (supposed_total_bytes_sent_string != NULL)
      {
        supposed_total_bytes_sent_int = atoi(supposed_total_bytes_sent_string);
        if ( (supposed_total_bytes_sent_int-strlen(buffer)) < total)
        {
          continue;
        }
      }

      //Check if page is content-type: text/html
      reti3 = regcomp(&regex3, "Content-Type:",REG_EXTENDED);
      int continue_flag = 1;
      char* content_type;

      for (j=0; j<=i-1; j++)
      {
        reti3 = regexec(&regex3, http_fields[j], 0 , NULL, 0);
        if (!reti3)
        {
          content_type = (char*)malloc(sizeof(char) * (strlen(http_fields[j])+1));
          sscanf(content_type, "Content-Type: %s", http_fields[j]);
          if (strcmp(content_type, "text/html")!=0) {
            continue;
          }
          free(content_type);
        }
      }
      // URL processing
      tidyBufAppend(&docbuf, buffer, sizeof(buffer)-1);

      int err;

      err = tidyParseBuffer(tdoc, &docbuf);
      if (err >= 0) {
        err = tidyCleanAndRepair(tdoc);
        if (err >= 0) {
          err = tidyRunDiagnostics(tdoc);
          if (err >= 0) {
            dumpNode(tdoc, tidyGetRoot(tdoc), host, path, unique_urls);
          }
        }
      }

      tidyBufFree(&docbuf);
      tidyBufFree(&tidy_errbuf);
      tidyRelease(tdoc);
      close(sockfd);
      num_parsed_urls++;
      current_url_index++;
    }
}
