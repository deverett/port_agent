/*******************************************************************************
 * Class: InstrumentRSNConnection
 * Filename: observatory_connection.h
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

#ifndef __INSTRUMENT_RSN_CONNECTION_H_
#define __INSTRUMENT_RSN_CONNECTION_H_

#include "port_agent/connection/connection.h"
#include "network/tcp_comm_socket.h"

using namespace std;
using namespace network;

namespace port_agent {
    class InstrumentRSNConnection : public Connection {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
            InstrumentRSNConnection();
            InstrumentRSNConnection(const InstrumentRSNConnection &rhs);
            virtual ~InstrumentRSNConnection();
            
            void initialize();
            void copy(const InstrumentRSNConnection &copy);
            
            /* Operators */
            InstrumentRSNConnection & operator=(const InstrumentRSNConnection &rhs);

            /* Accessors */
            
            CommBase *dataConnectionObject() { return &m_oDataSocket; }
            CommBase *commandConnectionObject() { return &m_oCommandSocket; }
            
            PortAgentConnectionType connectionType() { return PACONN_INSTRUMENT_RSN; }
            
            // Custom configurations for the observatory connection
            void setDataPort(uint16_t port);
            void setCommandPort(uint16_t port);
            void setDataHost(const string &host);
            void setCommandHost(const string &host);
            
            const string & dataHost() { return m_oDataSocket.hostname(); }
            uint16_t dataPort() { return m_oDataSocket.port(); }
            uint16_t commandPort() { return m_oCommandSocket.port(); }
            bool connected();
            bool disconnect();
            
            /* Query Methods */
            
            // Do we have complete configuration information for each
            // socket connection?
            bool dataConfigured();
            bool commandConfigured();
            
            // Has the connection been initialized (is it listening?)
            bool dataInitialized();
            bool commandInitialized();
            
            // Has a connection been made?
            bool dataConnected();
            bool commandConnected();
            
            /* Commands */
            
            // Initialize sockets
            void initializeDataSocket();
            void initializeCommandSocket();

            // Send break condition for duration (milliseconds)
            bool sendBreak(const uint32_t duration);

            // Turn DIGI timestamping on
            bool setTimestampBinaryOn(void);

            // Generic method for sending commands to the command port
            //bool sendCommand(const char *buffer);
            bool sendCommand(ostringstream &command);

        protected:

        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
            
        private:
            TCPCommSocket m_oDataSocket;
            TCPCommSocket m_oCommandSocket;
            
    };
}

#endif //__INSTRUMENT_RSN_CONNECTION_H_
