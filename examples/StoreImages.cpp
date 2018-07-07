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
// It will read the system time and convert it into the human readable format.
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

// This code is borrowed from -- https://stackoverflow.com/a/478960/5095879
// It will return the output of a system command executed on terminal
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

// Thread to communicate with GUI which send signal to start/stop sensor, grab/pause,
// images etc.
void *StoreImages(void *ptr){
    ThreadData *inThData = (ThreadData *)ptr;
	std::string destPath = inThData->dirDestination;	// Save destination path

    //std::cout << inThData->dirDestination.c_str() << std::endl; 
    int sockfd, portno, n = 0;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int boolval = 1;            // for a socket option
    
    // Creates socket. Connection based.
    // sockfd = socket(AF_INET, SOCK_STREAM, 0);       //TCP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);       //UDP
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
    // TCP -- listen for connections
    // listen(sockfd, 5);

    // change socket permissions to allow broadcast
   if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
    {
        printf("error setting socket options\n");
        exit(-1);
    }


    // Client related information
    clilen = sizeof(cli_addr);   // size of structure
    char buffer[MSG_SIZE];
    bzero(buffer,MSG_SIZE);

    while (1){
        // TCP -- Blocks until a client connects to the server
        // newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        // if (newsockfd < 0)
        //     error("ERROR on accept");

        // Read data from the client
        bzero(buffer, MSG_SIZE);
        // TCP style
        // n = read(newsockfd,buffer,MSG_SIZE-1); // recvfrom() could be used

        // UDP Style
        n = recvfrom(sockfd, buffer, MSG_SIZE, 0, (struct sockaddr *)&cli_addr, &clilen);
        if (n < 0)
            error("ERROR reading from socket");
        std::cout << "Here is the message: %s\n" << buffer << std::endl;

        // Check the message first.
        if (strncasecmp(buffer, "Start", 5) == 0){
            // Create a folder name, that is derived from the current date, and store it 
			// in given path name.
            if(inThData->dirDestination != ""){
                inThData->dirDestination = destPath + "/" + currentDateTime();
            }else{
                inThData->dirDestination = currentDateTime();
            }
            std::cout << "Image storing flag is " << inThData->store_Images << std::endl;
            std::cout << "The folder name is " << inThData->dirDestination << std::endl;
            std::cout << "The frame count starts from - " << inThData->frameCount \
						<< std::endl;
            
			// Folder creation -- If the folder creation fails then inform the same to the
			// GUI and don't set the store-image flag.
			int cd_ret = CreateDirectory(inThData->dirDestination);		
            bzero(buffer, MSG_SIZE);			// Clear the buffer before adding new message
			if (cd_ret == 0){
				inThData->store_Images = true;	// Turn ON the storage flag
				inThData->frameCount = 1;		// Start the image number from 1
				strcpy(buffer, "Created folder -- ");
				strcat(buffer, inThData->dirDestination.c_str());
			}else{
				strcpy(buffer, "Unable to create a folder");
			}
        }else if(strncasecmp(buffer, "Stop", 4) == 0){
            inThData->store_Images = false;
            std::cout << "Image storing flag is " << inThData->store_Images
                      << std::endl;
            // Send an acknowledgement - Number of images grabbed for each type.
            bzero(buffer, MSG_SIZE);
            char sysCmd[200];
            sprintf(sysCmd, "ls %s/depth* | wc", inThData->dirDestination.c_str());
            std::string cmdOutput = exec(sysCmd);
            strcpy(buffer, "Image grabbing done.\n");
            strcat(buffer, "Total image saved for each type: ");
            strcat(buffer, cmdOutput.c_str());
        }else{
            // Send an error message.
            bzero(buffer, MSG_SIZE);
            strcpy(buffer, "The message should be either \"Start\" or \"Stop\"");
        }

        // TCP way --- Send acknowledgement to the client
        // n = write(newsockfd, buffer, strlen(buffer));
        // close(newsockfd);
        // signal(SIGCHLD,SIG_IGN);   // to avoid zombie problem

        // UPD way -- Send acknowledgement to the client
        n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cli_addr, clilen);
    }  // end of while

    close(sockfd);
    pthread_exit(0);
}

