#include "StoreImages.h"

// Create the directory to store the images.
int CreateDirectory(std::string dirDest){
    int mkdRet = mkdir(dirDest.c_str(), 0777);
    if (mkdRet == 0){
//        std::cout << "The return value is: " << mkdRet << std::endl;
        std::cout << "The \"" << dirDest.c_str() << "\"is created successfully" << std::endl;
    }
    else{
//        std::cout << "The return value is: " << mkdRet << std::endl;
        std::cout << "The folder \"" << dirDest.c_str() << "\" couldn't be created." << std::endl;
    }

	return mkdRet;
}

// Error message 
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// This code is borrowed from -- http://stackoverflow.com/a/10467633/5095879
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y_%m_%d-%H_%M", &tstruct);

    return buf;
}

// Thread to communicate with GUI which send signal to start/stop sensor, grab/pause,
// images etc.
void *StoreImages(void *ptr){
    ThreadData *inThData = (ThreadData *)ptr;
    //std::cout << inThData->dirDestination.c_str() << std::endl; 
    int sockfd, newsockfd, portno, n = 0;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // Creates socket. Connection based.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // fill in fields
    // bzero((char *) &serv_addr, sizeof(serv_addr));
    memset(&serv_addr, 0, sizeof(serv_addr));
    //portno = atoi(argv[1]);                   // get port number from input
    portno = 5432;                              // get port number from input
    serv_addr.sin_family = AF_INET;             // symbol constant for Internet domain
    serv_addr.sin_addr.s_addr = INADDR_ANY;     // IP address of the machine on which
                                                // the server is running
    serv_addr.sin_port = htons(portno);         // port number

    // binds the socket to the address of the host and the port number
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    // listen for connections
    listen(sockfd, 5);

    // Client related information
    clilen = sizeof(cli_addr);   // size of structure
    char buffer[MSG_SIZE];
    bzero(buffer,MSG_SIZE);

    while (1){
        // Blocks until a client connects to the server
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        // Read data from the client
        bzero(buffer, MSG_SIZE);
        n = read(newsockfd,buffer,MSG_SIZE-1); // recvfrom() could be used
        if (n < 0)
            error("ERROR reading from socket");
        std::cout << "Here is the message: %s\n" << buffer << std::endl;

        // Check the message first.
        if (strncasecmp(buffer, "Start", 5) == 0){
            // Create a folder name, that is derived from the current date, and store it in
			// given path name.
            if(inThData->dirDestination != ""){
                inThData->dirDestination = inThData->dirDestination + "/" + currentDateTime();
            }else{
                inThData->dirDestination = currentDateTime();
            }
            std::cout << "Image storing flag is " << inThData->store_Images << std::endl;
            std::cout << "The folder name is " << inThData->dirDestination << std::endl;
            
			// Folder creation -- If the folder creation fails then inform the same to the
			// GUI and don't set the store-image flag.
			int cd_ret = CreateDirectory(inThData->dirDestination);		
			bzero(buffer, MSG_SIZE);		// Clear the buffer before adding new message
			if (cd_ret == 0){
				inThData->store_Images = true;
				strcpy(buffer, "Created folder -- ");
				strcat(buffer, inThData->dirDestination.c_str());
			}else{
				strcpy(buffer, "Unable to create a folder");
			}
        }else if(strncasecmp(buffer, "Stop", 4) == 0){
            inThData->store_Images = false;
            std::cout << "Image storing flag is " << inThData->store_Images << std::endl;
            // Send an acknowledgement.
            bzero(buffer, MSG_SIZE);
            strcpy(buffer, "Image grabbing process has stopped.");
        }else{
            // Send an error message.
            bzero(buffer, MSG_SIZE);
            strcpy(buffer, "The message should be either \"Start\" or \"Stop\"");
        }

        // Send acknowledgement to the client
        n = write(newsockfd, buffer, strlen(buffer));
        //close(newsockfd);
        //signal(SIGCHLD,SIG_IGN);   // to avoid zombie problem
    }  // end of while

    close(sockfd);
    pthread_exit(0);
}

