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
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy of the packet data.
 *
 * Parameters:
 *   copy - rhs object to copy
 ******************************************************************************/
InstrumentRSNConnection::InstrumentRSNConnection(const InstrumentRSNConnection& rhs) {
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
 *   True if we have both the data and command connections
 ******************************************************************************/
bool InstrumentRSNConnection::connected() {
    return m_oDataSocket.connected() && m_oCommandSocket.connected();
}

/******************************************************************************
 * Method: disconnect
 * Description: Disconnect from both the data and the command ports on the RSN
 * DIGI
 *
 * Return:
 *   True if we have disconnected from both the data and command connections
 ******************************************************************************/
bool InstrumentRSNConnection::disconnect() {
    return m_oDataSocket.disconnect() && m_oCommandSocket.disconnect();
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
 * Description: NOOP
 ******************************************************************************/
void InstrumentRSNConnection::initializeCommandSocket() {
    m_oCommandSocket.initialize();
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

    if (commandConfigured() && bAllGood && !commandConnected()) {
        LOG(DEBUG) << "initialize command socket";
        initializeCommandSocket();
    }
}

/******************************************************************************
 * Method: sendBreak
 * Description: Send a break condition for the given duration.
 ******************************************************************************/
bool InstrumentRSNConnection::sendBreak(const uint32_t iDuration) {
    bool bReturnCode = true;


    // build the break command.

    std::ostringstream ssCommand;
    ssCommand << "break " << iDuration;


    if (!sendCommand(ssCommand)) {
        LOG(ERROR) << "Failed to send break.";
        bReturnCode = false;
    }

    return bReturnCode;
}

/******************************************************************************
 * Method: sendBreak
 * Description: Send a break condition for the given duration.
 ******************************************************************************/
bool InstrumentRSNConnection::setTimestampBinaryOn(void) {
    bool bReturnCode = true;


    // build the break command.

    std::ostringstream sCommand;
    sCommand << "timestamping 2";

    if (!sendCommand(sCommand)) {
        LOG(ERROR) << "Failed to send break.";
        bReturnCode = false;
    }

    return bReturnCode;
}

/******************************************************************************
 * Method: sendBreak
 * Description: Send a break condition for the given duration.
 ******************************************************************************/
//bool InstrumentRSNConnection::sendCommand(const char* buffer) {
bool InstrumentRSNConnection::sendCommand(std::ostringstream &ssCommand) {
    bool bReturnCode = true;

    // convert the command to a char array
    int iSize = ssCommand.tellp();
    string sCommand = ssCommand.str();
    //char* pCommand = sCommand.c_str();

    LOG(INFO) << "Sending command: " << sCommand << "; length: " << sCommand.length();

    //if (!m_oCommandSocket.writeData(pCommand, iSize)) {
    if (!m_oCommandSocket.writeData(sCommand.c_str(), sCommand.length())) {
        LOG(ERROR) << "Failed to send command: " << sCommand;
        bReturnCode = false;
    }

    // question: how do i look at the returned data?

    return bReturnCode;
}

