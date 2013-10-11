/*******************************************************************************
 * Class: TcpCommListener
 * Filename: tcp_comm_listener.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manage a TCP listener.
 *
 * Usage:
 *
 * TCPCommListener ts;
 *
 * // Statically assign the port.  If not assigned then us a random port.
 * ts.setPort(1024);
 *
 * // Enable blocking connections. Default is non-blocking
 * ts.setBlocking(true);
 *
 * // Initialize the server
 * ts.initalize();
 *
 * // Get the port the server is actually listening on.  Useful when using
 * // random ports.
 * uint16_t port = ts.getListenPort();
 * 
 * // Accept client connections
 * ts.acceptClient();
 *
 * // Read data from the client. Honors the blocking flag set earlier.
 * char buffer[128];
 * int bytes_read = ts.readData(buffer, 128);
 *
 * // Write data to the client.
 * int bytes_written = ts.writeData("Hello World", strlen("Hello World"));
 *
 * // When using non-blocking you may want to use a select read loop to monitor
 * // the file descriptors.  They are exposed via accessors
 * int serverFD = ts.getServerFD();
 * int clientFD = ts.getServerFD();
 ******************************************************************************/

#include "tcp_comm_listener.h"
#include "common/util.h"
#include "common/logger.h"
#include "common/exception.h"
#include "common/timestamp.h"

#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


using namespace std;
using namespace logger;
using namespace network;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: Constructor
 * Description: Default constructor.
 ******************************************************************************/
TCPCommListener::TCPCommListener() : CommBase() {
    m_iPort = 0;
	    
    m_pServerFD = 0;
    m_pClientFD = 0;
}


/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor.
 ******************************************************************************/
TCPCommListener::TCPCommListener(const TCPCommListener &rhs) : CommBase(rhs) {
	LOG(DEBUG) << "TCPCommListener Copy CTOR!!";
    m_iPort = rhs.m_iPort;
	    
    m_pServerFD = rhs.m_pServerFD;
    m_pClientFD = rhs.m_pClientFD;
}


/******************************************************************************
 * Method: Destructor
 * Description: destructor.
 ******************************************************************************/
TCPCommListener::~TCPCommListener() {
    LOG(DEBUG) << "TCPCommListener DTOR";
	disconnect();
}

/******************************************************************************
 * Method: copy
 * Description: return a new object deep copied.
 ******************************************************************************/
CommBase * TCPCommListener::copy() {
	return new TCPCommListener(*this);
}

/******************************************************************************
 * Method: assignment operator
 * Description: overloaded assignment operator.
 ******************************************************************************/
TCPCommListener & TCPCommListener::operator=(const TCPCommListener &rhs) {
	return *this;
}

/******************************************************************************
 * Method: equality operator
 * Description: overloaded equality operator.
 ******************************************************************************/
bool TCPCommListener::operator==(TCPCommListener &rhs) {
        return compare(&rhs);
}

/******************************************************************************
 * Method: compare
 * Description: compare objects
 ******************************************************************************/
bool TCPCommListener::compare(CommBase *rhs) {
		if(rhs->type() != COMM_TCP_LISTENER)
		    return false;
        return m_iPort == ((TCPCommListener *)rhs)->m_iPort;
}


/******************************************************************************
 * Method: isConfigured
 * Description: Nothing to do here.
 ******************************************************************************/
bool TCPCommListener::isConfigured() {
    return true;
}

/******************************************************************************
 * Method: disconnect
 * Description: Disconnect a client and server
 * Exceptions:
 *   SocketMissingConfig
 ******************************************************************************/
bool TCPCommListener::disconnect() {
    LOG(DEBUG) << "Shutdown server";
    
    disconnectClient(true);
    disconnectServer();
    
    return true;
}

/******************************************************************************
 * Method: disconnectClient
 * Description: Disconnect a client
 ******************************************************************************/
bool TCPCommListener::disconnectClient(bool server_shutdown) {
    if(connected()) {
        LOG(DEBUG2) << "Disconnecting client";
	    //shutdown(m_pClientFD,2);
	    close(m_pClientFD);
	    m_pClientFD = 0;
    }
	
	if(!server_shutdown) {
		LOG(DEBUG) << "Re-initalize tcp listener";
	    initialize();
	}
    
    return true;
}
/******************************************************************************
 * Method: disconnectServer
 * Description: Disconnect a server
 ******************************************************************************/
bool TCPCommListener::disconnectServer() {
    if (listening()) {
        LOG(DEBUG) << "Closing server connection FD: " << m_pServerFD;
	    //shutdown(m_pServerFD,2);
	    close(m_pServerFD);
	    m_pServerFD = 0;
    }
    
    return true;
}

/******************************************************************************
 * Method: acceptClient
 * Description: Accept a client connection and create a new FD
 * Exceptions:
 *   SocketNotInitialized
 *   SocketAlreadyConnected
 ******************************************************************************/
bool TCPCommListener::acceptClient(bool persistent) {
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int newsockfd;

    LOG(DEBUG) << "persistent: " << persistent;

    if (!listening())
        throw SocketNotInitialized();

    if (connected()) {
        LOG(ERROR) << "Already connected!.";

        clilen = sizeof(cli_addr);
        newsockfd = accept(m_pServerFD, 
                    (struct sockaddr *) &cli_addr, 
                    &clilen);
		if (newsockfd) {
		    close(newsockfd);
		}
        throw SocketAlreadyConnected();
	}

    LOG(DEBUG) << "accepting client connection from FD: " << m_pServerFD;
     
    clilen = sizeof(cli_addr);
    newsockfd = accept(m_pServerFD, 
                (struct sockaddr *) &cli_addr, 
                &clilen);
     
    LOG(DEBUG) << "client FD: " << newsockfd;
    
    if (newsockfd < 0) {
	    if(errno == EAGAIN  || errno == EWOULDBLOCK) {
	        LOG(DEBUG) << "Non-blocking error ignored: " << strerror(errno) << "(" << errno << ")";
    	    return false;
	    }
	    else {
                throw SocketConnectFailure(strerror(errno));
	    }
    }
    
    // Set the client to non blocking if needed
    if(! blocking()) {
	    LOG(DEBUG) << "set client non-blocking";
	    fcntl(newsockfd, F_SETFL, O_NONBLOCK);
	    LOG(DEBUG) << "Set to non-blocking";
    }
    
    LOG(DEBUG) << "Storing new FD: " << newsockfd;
	m_pClientFD = newsockfd;
	
	if (!persistent) {
	    LOG(DEBUG) << "Disconnect server";
	    disconnectServer();
	}
	
    return true;
}

/******************************************************************************
 * Method: getListenPort
 * Description: Return the port the server is actually listening on.
 * Exceptions:
 *   SocketNotInitialized
 ******************************************************************************/
uint16_t TCPCommListener::getListenPort() {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    uint16_t iPort;
    
    LOG(DEBUG) << "Fetch listen port";
    
    if (!listening()) {
        LOG(DEBUG) << "Listener not currently listening";
	    return 0;
    }

    if (getsockname(m_pServerFD, (struct sockaddr *)&sin, &len) == -1)
        throw SocketConnectFailure(strerror(errno));

    iPort = ntohs(sin.sin_port);
    LOG(DEBUG) << "Port from FD: " << m_pServerFD << " is: " << iPort;
        
    return iPort;
}

/******************************************************************************
 * Method: initalize
 * Description: Setup a TCP listener
 * Exceptions:
 *   SocketMissingConfig
 *   SocketConnectFailure
 ******************************************************************************/
bool TCPCommListener::initialize() {
	int fflags;
	int optval;
	struct sockaddr_in serv_addr;
	struct hostent *server;
    int retval;
	int newsock;
	Timestamp ts;
	int bind_result = -1;
	
	LOG(DEBUG) << "TCP Listener initialize()";

	if(!isConfigured())
		throw SocketMissingConfig("missing inet port");

	LOG(DEBUG2) << "Creating INET socket";
	newsock = socket(AF_INET, SOCK_STREAM, 0);

	if(!newsock)
		throw SocketCreateFailure("socket create failure");

	optval = 1;
	if (setsockopt(newsock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1) {
	    throw SocketCreateFailure("setsockopt SO_REUSADDR failure");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(m_iPort);

	LOG(DEBUG2) << "bind to port " << m_iPort;
	while (bind_result < 0) {
	    bind_result = bind(newsock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
		
		if(bind_result < 0) {
            LOG(ERROR) << "Failed to bind: " << strerror(errno) << "(" << errno << ")";
			
			// Retry on address in use errors if we haven't exceeded timeout.  Otherwise
			// raise an exception.
			if(errno == EADDRINUSE && ts.elapseTime() < TCP_BIND_TIMEOUT) {
				LOG(INFO) << "Waiting for port to freeup.  retrying bind.";
			}
			else {
		        throw SocketConnectFailure(strerror(errno));
			}
			
		    sleep(1);
	    }
	}
	    
	LOG(DEBUG2) << "Starting server";
	retval = listen(newsock, 0);
	LOG(DEBUG3) << "listen return value: " << retval;
	
	if (retval < 0)
	    if(errno != EINPROGRESS ) // ignore EINPROGRESS error because we are NON-Blocking
                throw(SocketConnectFailure(strerror(errno)));

	if(! blocking()) {
		LOG(DEBUG3) << "set server socket non-blocking";
		fcntl(newsock, F_SETFL, O_NONBLOCK);
		int opts = fcntl(newsock, F_GETFL);
		LOG(DEBUG3) << "fd: " << hex << newsock << " "
		            << "sock opts: " << hex << opts << " "
		            << "non block flag: " << hex << O_NONBLOCK;
	}
	
	LOG(DEBUG2) << "storing new fd: " << newsock;
	m_pServerFD = newsock;
	
	// Fail if we tried to bind to a specific port, but it gave us a random
	// port instead.
	if(m_iPort && getListenPort() != m_iPort)
	    throw SocketConnectFailure("bind to port failed");
	
	LOG(DEBUG2) << "startup complete.  host port " << getListenPort();
	return true;
}


/******************************************************************************
 * Method: write
 * Description: write a number of bytes to the socket connection.  Currently we
 * try to write three times before we fail.  We might want to update this retry
 * so that it keeps retrying if it see progress being made?
 *
 * Parameters:
 *   buffer - the data to write
 *   size - the size of the buffer array
 * Return:
 *   returns the actual number of bytes written.
 * Exceptions:
 *   SocketNotInitialized
 *   SocketNotConnected
 *   SocketWriteFailure
 ******************************************************************************/
uint32_t TCPCommListener::writeData(const char *buffer, const uint32_t size) {
    int bytesWritten = 0;
    int bytesRemaining = size;
    int count;

    if(! connected()) {
		LOG(DEBUG) << "Socket (FD: " << m_pClientFD << ") not connected";
		return 0;
    }
    
    while( bytesRemaining > 0 ) {
        LOG(DEBUG) << "WRITE DEVICE: " << buffer << "FD: " << m_pClientFD;
        count = write(m_pClientFD, buffer + bytesWritten, bytesRemaining );
        LOG(DEBUG1) << "bytes written: " << count << " remaining: " << bytesRemaining;
        if(count < 0) {
            LOG(ERROR) << strerror(errno) << "(errno: " << errno << ")";
            throw(SocketWriteFailure(strerror(errno)));
        }

        bytesWritten += count;
        bytesRemaining -= count;

        LOG(DEBUG2) << "wrote bytes: " << count << " bytes remaining: " << bytesRemaining;
    }

    return bytesWritten;
}


/******************************************************************************
 * Method: read
 * Description: read a number of bytes to the socket connection.
 *
 * Parameters:
 *   buffer - where to store the read data
 *   size - max number of bytes to read
 * Return:
 *   returns the actual number of bytes read.
 * Exceptions:
 *   SocketNotInitialized
 *   SocketNotConnected
 *   SocketReadFailure
 ******************************************************************************/
uint32_t TCPCommListener::readData(char *buffer, const uint32_t size) {
    int bytesRead = 0;

    if(! connected()) {
	    LOG(ERROR) << "Socket Not Connected in readData";
        throw(SocketNotConnected("in TCPCommListener readData"));
	}
    
    if ((bytesRead = read(m_pClientFD, buffer, size)) < 0) {
        if (errno == EAGAIN || errno == EINPROGRESS) {
            LOG(DEBUG2) << "Error Ignored: " << strerror(errno);
        } else if( errno == ETIMEDOUT ) {
            LOG(DEBUG) << " -- socket read timeout. disconnecting client FD:" << m_pClientFD;
            disconnectClient();
        } else {
            LOG(ERROR) << "bytes read: " << bytesRead << " read_device: " << strerror(errno) << "(errno: " << errno << ")";
            throw(SocketReadFailure(strerror(errno)));
        }

        LOG(DEBUG2) << "read bytes: " << bytesRead;
    }
    else if(bytesRead == 0) {
        LOG(INFO) << " -- Device connection closed; zero bytes received.";
        disconnectClient();
    }
    else
        LOG(DEBUG) << "READ DEVICE: " << buffer;

    return bytesRead < 0 ? 0 : bytesRead;
}
