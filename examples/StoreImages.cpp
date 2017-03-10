#include "StoreImages.h"

// Create the directory to store the images.
void CreateDirectory(std::string dirDest){
    char cmdName[100];
    sprintf(cmdName, "mkdir %s", dirDest.c_str());
    int sysRet = system(cmdName);
    if (sysRet == 0){
        std::cout << "The \"" << dirDest.c_str() << "\"is created successfully" << std::endl;
    }
    else{
        std::cout << "The \"" << dirDest.c_str() << "\" already exists." << std::endl;
    }
}

// Thread to communicate with GUI which send signal to start/stop sensor, grab/pause images,
// etc.
void *StoreImages(void *ptr){
    ThreadData *inThData = (ThreadData *)ptr;
    std::cout << inThData->dirDestination.c_str() << std::endl; 
    // Create a socket and bind with a predefined port.

    while(0){}

	// Read the bytes coming through the socket
	inThData->store_Images = true;
    std::cout << "Now stopped grabbing images." << std::endl;
    // Create the directory.
    CreateDirectory(inThData->dirDestination);

	pthread_exit(0);
}

