/*******************************************************************************
 * Class: InstrumentRSNConnection
 * Filename: instrument_tcp_connection.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manages the socket connection between the observatory and the port agent.
 * This interface consists of a TCP listener on the data port and command port
 * setup in non-blocking mode.
 *
 * Usage:
 *
 * InstrumentRSNConnection connection;
 *
 * connection.setDataPort(4001);
 *
 * // Is the data port configured
 * connection.dataConfigured();
 *
 * // This is a noop for this method.  There is nothing to initialize
 * connection.initialize();
 *
 * // Always true for this connection type.
 * connection.dataInitialized();
 *
 * // Is the data port connected
 * connection.dataConnected();
 *
 * // Always false for this connection type
 * connection.commandConnected();
 *
 * // Get a pointer tcp data listener object
 * TCPCommListener *data = connection.dataConnectionObject();
 *    
 * // Always returns null for this connection type
 * TCPCommListener *command = connection.commandConnectionObject();
 *    
 ******************************************************************************/
#include <string>
#include <string.h>
#include "instrument_rsn_connection.h"
#include "common/util.h"
#include "common/logger.h"
#include "common/exception.h"
#include "network/tcp_comm_listener.h"

using namespace std;
using namespace logger;
using namespace network;
using namespace port_agent;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/
/******************************************************************************
 * Method: Constructor
 * Description: Default constructor.  Is likely never called, but wanted to
 *              define it explicitly.
 ******************************************************************************/
InstrumentRSNConnection::InstrumentRSNConnection() : Connection() {
    m_bTimestampBinaryOn = false;
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy of the packet data.
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
InstrumentRSNConnection::InstrumentRSNConnection(const InstrumentRSNConnection& rhs) {
    m_bTimestampBinaryOn = false;
    copy(rhs);
}

/******************************************************************************
 * Method: Destructor
 * Description: free up our dynamically created packet data.
 ******************************************************************************/
InstrumentRSNConnection::~InstrumentRSNConnection() {
}

/******************************************************************************
 * Method: Assignemnt operator
 * Description: Deep copy
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
InstrumentRSNConnection & InstrumentRSNConnection::operator=(const InstrumentRSNConnection &rhs) {
    copy(rhs);
    return *this;
}


/******************************************************************************
 * Method: copy
 * Description: Copy data from one packet object into this one.  Ensuring we
 * deep copy when needed.
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
void InstrumentRSNConnection::copy(const InstrumentRSNConnection &copy) {
    m_oDataSocket = copy.m_oDataSocket;
    m_oCommandSocket = copy.m_oCommandSocket;
    m_bTimestampBinaryOn = copy.m_bTimestampBinaryOn;
}

/******************************************************************************
 * Method: setDataPort
 * Description: Set the port.  If we are already connected then we need to
 * disconnect and reconnect to the new port.
 ******************************************************************************/
void InstrumentRSNConnection::setDataPort(uint16_t port) {
    uint16_t oldPort = m_oDataSocket.port();
    m_oDataSocket.setPort(port);
    
    if(m_oDataSocket.connected() && m_oDataSocket.port() != oldPort) {
	m_oDataSocket.initialize();
    }
}

/******************************************************************************
 * Method: setCommandPort
 * Description: Set the command port.  If we are already connected then we need
 * to disconnect and reconnect to the new port.
 ******************************************************************************/
void InstrumentRSNConnection::setCommandPort(uint16_t port) {
    uint16_t oldPort = m_oCommandSocket.port();
    m_oCommandSocket.setPort(port);

    if(m_oCommandSocket.connected() && m_oCommandSocket.port() != oldPort) {
        m_oCommandSocket.initialize();
    }
}

/******************************************************************************
 * Method: setDataHost
 * Description: Set the host.  If we are already connected then we need to
 * disconnect and reconnect to the new host.
 ******************************************************************************/
void InstrumentRSNConnection::setDataHost(const string & host) {
    string oldhost = m_oDataSocket.hostname();
    m_oDataSocket.setHostname(host);
    
    if(m_oDataSocket.connected() && m_oDataSocket.hostname() != oldhost) {
        m_oDataSocket.initialize();
    }
}

/******************************************************************************
 * Method: setCommandHost
 * Description: Set the host.  If we are already connected then we need to
 * disconnect and reconnect to the new host.
 ******************************************************************************/
void InstrumentRSNConnection::setCommandHost(const string & host) {
    string oldhost = m_oCommandSocket.hostname();
    m_oCommandSocket.setHostname(host);

    if(m_oCommandSocket.connected() && m_oCommandSocket.hostname() != oldhost) {
        m_oCommandSocket.initialize();
    }
}

/******************************************************************************
 * Method: connected
 * Description: Are we connected to both ports on the RSN DIGI?
 *
 * Return:
 *   True if we are connected to the data RSN DIGI data port.
 *   False if not connected to the RSN DIGI data port.
 ******************************************************************************/
bool InstrumentRSNConnection::connected() {
    return m_oDataSocket.connected();
}

/******************************************************************************
 * Method: disconnect
 * Description: Disconnect from both the data and the command ports on the RSN
 * DIGI
 *
 * Return:
 *   True if we have disconnected from the data port.
 *   False otherwise (don't think that ever happens).
 ******************************************************************************/
bool InstrumentRSNConnection::disconnect() {
    return m_oDataSocket.disconnect();
}

/******************************************************************************
 * Method: dataConfigured
 * Description: Do we have enough configuration information to initialize the
 * data socket?
 *
 * Return: 
 *   True if we have enough configuration information
 ******************************************************************************/
bool InstrumentRSNConnection::dataConfigured() {
    return m_oDataSocket.isConfigured();
}

/******************************************************************************
 * Method: commandConfigured
 * Description: Do we have enough configuration information to initialize the
 * command socket?
 *
 * Return: 
 *   True if we have enough configuration information
 ******************************************************************************/
bool InstrumentRSNConnection::commandConfigured() {
    return m_oCommandSocket.isConfigured();
}

/******************************************************************************
 * Method: dataInitialized
 * Description: Currently returning commandConfigured(). Might need to wait
 * until we've successfully initialized the RSN DIGI to use binary timestamp.
 *
 * Return:
 *   True if configured.
 ******************************************************************************/
bool InstrumentRSNConnection::dataInitialized() {
    return dataConfigured();
}

/******************************************************************************
 * Method: commandInitialized
 * Description: Currently returning commandConfigured(). Might need to wait
 * until we've successfully initialized the RSN DIGI to use binary timestamp.
 *
 * Return:
 *   False
 ******************************************************************************/
bool InstrumentRSNConnection::commandInitialized() {
    return commandConfigured();
}

/******************************************************************************
 * Method: dataConnected
 * Description: Is a client connected to the data socket connection
 *
 * Return:
 *   True if the data socket is connected
 ******************************************************************************/
bool InstrumentRSNConnection::dataConnected() {
    return m_oDataSocket.connected();
}

/******************************************************************************
 * Method: commandConnected
 * Description: Alwasy false because there is no command interface for this
 * connection type.
 *
 * Return:
 *   False
 ******************************************************************************/
bool InstrumentRSNConnection::commandConnected() {
    return m_oCommandSocket.connected();
}

/******************************************************************************
 * Method: initializeDataSocket
 * Description: Initialize the data socket
 ******************************************************************************/
void InstrumentRSNConnection::initializeDataSocket() {
    m_oDataSocket.initialize();
}

/******************************************************************************
 * Method: initializeCommandSocket
 * Description: Initialize the command socket to the DIGI.  The DIGI sends an
 * initial banner & prompt that we need to read (mainly to clear the buffer so
 * that we can successfully send and other commands and read their responses.
 ******************************************************************************/
void InstrumentRSNConnection::initializeCommandSocket() {

    /*
     * Initialize the command socket. Every time we do, we will get the banner,
     * so we need to make sure we read that out of the socket.  Then we need to
     * turn binary timestamping on.  We shouldn't move to connected state if
     * timestamping isn't on.
     *
     * Also, we need to initialize the socket here, because every command
     * operation calls this method.  But we can't close the socket here,
     * because we'll close it right out from under the method that is issueing
     * the command.  So we close the connection in the command methods.
     */

    m_oCommandSocket.initialize();

    /*
     * The DIGI sends an initial banner & prompt; make sure we get that.  Also
     * turn binary timestamping on.
     */

    string sResponse = "OOI - Digi Command Interface\r\ntype help for command information\r\n";
    if ((readCommandResponse(sResponse) == false) ||
        (setTimestampBinaryOn() == false)) {
        m_bTimestampBinaryOn = false;
    }
    else {
        m_bTimestampBinaryOn = true;
        LOG(INFO) << "Command socket FD: " << m_oCommandSocket.getSocketFD() << " initialized.";
    }

}

/******************************************************************************
 * Method: initialize
 * Description: Initialize any uninitialized sockets if they are configured.
 ******************************************************************************/
void InstrumentRSNConnection::initialize() {
    bool bAllGood = true;

    if (!dataConfigured()) {
        bAllGood = false;
        LOG(WARNING) << "Data port not configured. Not initializing";
    }
	
    if (!commandConfigured()) {
        bAllGood = false;
        LOG(WARNING) << "Command port not configured. Not initializing";
    }

    if (dataConfigured() && bAllGood && !dataConnected()) {
        LOG(DEBUG) << "initialize data socket";
        initializeDataSocket();
    } 

    /*
     * If all is still good, initialize the command connection only to
     * assert that we can initialize it (i.e., configured correctly)
     * and that we can turn binary timestamping on.
     */
    if (commandConfigured() && bAllGood && !commandConnected()) {
        LOG(DEBUG) << "initialize command socket";
        initializeCommandSocket();

        /*
         * Disconnect because we don't want to keep the command port open.
         * We can do that here because this is method isn't called each
         * time we do a command, but rather only when we initialize an
         * RSN connection.
         */

        m_oCommandSocket.disconnect();
    }
}

/******************************************************************************
 * Method: sendBreak
 * Description: Send a break condition for the given duration.
 *
 * Return boolean: true if successful (including reading command ack).
 *                 false if failure.
 ******************************************************************************/
bool InstrumentRSNConnection::sendBreak(const uint32_t iDuration) {
    bool bReturnCode = true;
    string sDuration = toString(iDuration);
    string sResponse = "Sending Serial Break " + sDuration + "(ms)\r\n\r\n";

    initializeCommandSocket();

    // build the break command.
    std::ostringstream ssCommand;
    ssCommand << "break " << iDuration << "\r\n";

    bReturnCode = sendCommand(ssCommand);
    if (true == bReturnCode) {
        bReturnCode = readCommandResponse(sResponse);
        if (false == bReturnCode) {
            LOG(ERROR) << "break command not acknowledged!";
        }
    }
    else {
        LOG(ERROR) << "sendCommand returned false.";
    }

    // Disconnect because we don't want to keep the command port open.
    m_oCommandSocket.disconnect();

    return bReturnCode;
}

/******************************************************************************
 * Method: setTimestampBinaryOn
 * Description: Send the timestamping command, set to binary.
 *
 * Return boolean: true if successful (including reading command ack).
 *                 false if failure.
 ******************************************************************************/
bool InstrumentRSNConnection::setTimestampBinaryOn(void) {
    bool bReturnCode = true;
    string sResponse = "Set Timestamping:On(binary)\r\n\r\n";

    // build the setTimestamp command: alwasy set to binary (2).
    std::ostringstream ssCommand;
    ssCommand << "timestamping 2\r\n";

    bReturnCode = sendCommand(ssCommand);
    if (true == bReturnCode) {
        bReturnCode = readCommandResponse(sResponse);
        if (false == bReturnCode) {
            LOG(ERROR) << "setTimestamp command not acknowledged!";
        }
        else {
            LOG(DEBUG) << "setTimestamp command acknowledged!";
        }
    }
    else {
        LOG(ERROR) << "sendCommand returned false.";
    }

    /*
     * We don't disconnect in this method, because this method is called
     * as part of the initialization for every command, and we'd
     * disconnect from under the intended command (ex. sendBreak).
     */

    return bReturnCode;
}

/******************************************************************************
 * Method: isTimestampBinaryOn
 * Description: Return the status of binary timestamping on RSN DIGI.
 *
 * Return boolean: true if on.
 *                 false if off.
 ******************************************************************************/
bool InstrumentRSNConnection::isTimestampBinaryOn(void) {

    return m_bTimestampBinaryOn;
}

/******************************************************************************
 * Method: sendCommand
 * Description: Send the given command to the DIGI, and read the echoed
 * response.
 *
 * Return boolean: true if successful (including reading command echo).
 *                 false if failure.
 ******************************************************************************/
bool InstrumentRSNConnection::sendCommand(std::ostringstream &ssCommand) {
    bool bReturnCode = true;

    // convert the command to a char array
    int iSize = ssCommand.tellp();
    string sCommand = ssCommand.str();

    LOG(INFO) << "Sending command: " << sCommand << "; length: " << sCommand.length();

    if (!m_oCommandSocket.writeData(sCommand.c_str(), sCommand.length())) {
        LOG(ERROR) << "Failed to send command: " << sCommand;
        bReturnCode = false;
    }
    else {
        bReturnCode = readCommandResponse(sCommand);
        if (false == bReturnCode) {
            LOG(ERROR) << "Command not echoed!";
        }
    }

    return bReturnCode;
}

/******************************************************************************
 * Method: sendBreak
 * Description: Read the given response from the DIGI.
 *
 * Return boolean: true if response is found.
 *                 false if timeout or response didn't match.
 ******************************************************************************/
bool InstrumentRSNConnection::readCommandResponse(const string &sResponse) {
    bool bResponseReceived = false;
    char* pBuffer = new char[1000];
    struct timespec sTimeDelay;

    LOG(DEBUG2) << "readCommandResponse looking for :" << sResponse;

    if (sResponse.length() > 0) {
        uint32_t bytes_read = 0;
        uint32_t total_bytes_read = 0;
        uint32_t iTimeout = 30; // 3 seconds
        while (bytes_read < sResponse.length() && iTimeout) {
            bytes_read = m_oCommandSocket.readData(&(pBuffer[total_bytes_read]), sResponse.length());
            total_bytes_read += bytes_read;
            LOG(DEBUG2) << "Received " << total_bytes_read << " bytes from command socket: " << pBuffer;

            sTimeDelay.tv_sec = 0; /* Number of seconds */
            sTimeDelay.tv_nsec = 100 * 1000000;   /*Number of nanoseconds */

            nanosleep(&sTimeDelay, NULL);
            iTimeout--;
        }

        if (0 >= iTimeout) {
            LOG(ERROR)  << "Timeout waiting for response: <" << sResponse << "> from DIGI";
        }
        else {
            // Get the see if the response matches
            if (!strncmp(sResponse.c_str(), pBuffer, sResponse.length())) {
                bResponseReceived = true;
            }
            else {
                LOG(ERROR) << "Received response: " << pBuffer << " did not match: " << sResponse;
            }
        }
    }

    return bResponseReceived;
}
