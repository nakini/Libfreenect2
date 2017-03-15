#ifndef __STOREIMAGES_INCLUDED__
#define __STOREIMAGES_INCLUDED__
#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>                   // Current time functionalities.
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>

#define MSG_SIZE 100			// message size

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

// Communicates with GUI process.
void *StoreImages(void *ptr);
// Creates directory in which images will be stored.
int CreateDirectory(std::string dirDest);
// Get the current date/time and convert into a string.
const std::string currentDateTime();
#endif
