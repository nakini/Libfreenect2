#ifndef __STOREIMAGES_INCLUDED__
#define __STOREIMAGES_INCLUDED__
#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#ifdef WIN32
  #include <winsock.h>         // For socket(), connect(), send(), and recv()
  typedef int socklen_t;
  typedef char raw_type;       // Type used for raw data on this platform
#else
  #include <sys/types.h>       // For data types
  #include <sys/socket.h>      // For socket(), connect(), send(), and recv()
  #include <netdb.h>           // For gethostbyname()
  #include <arpa/inet.h>       // For inet_addr()
  #include <unistd.h>          // For close()
  #include <netinet/in.h>      // For sockaddr_in
  typedef void raw_type;       // Type used for raw data on this platform
#endif
// Structure to hold the grab/pause flag and the name of the directory which will hold the
// stored IR/RGB images.
//typedef struct ThreadData ThreadData;
struct ThreadData{
    bool store_Images;		// Store the grabbed images
	std::string dirDestination;     // Directory to store the images.
};

void *StoreImages(void *ptr);
void CreateDirectory(std::string dirDest);
#endif
