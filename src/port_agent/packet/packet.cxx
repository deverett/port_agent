/*******************************************************************************
 * Class: Packet
 * Filename: packet.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Basic packet object for the port agent.  Packets are the communication
 * protocol for the port agent to outside interfaces via the data and command
 * port.  They are transmitted as binary datagrams with the following structure.
 *
 * This is the most basic type of packet, and typically isn't instantiated
 * directly, but rather by instantiating a subclass such as PortAgentPacket or
 * RSNPacket.
 * 
 *    
 ******************************************************************************/

#include "packet.h"
#include "common/util.h"
#include "common/logger.h"
#include "common/exception.h"
#include "common/timestamp.h"

#include <netinet/in.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cctype>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace packet;
using namespace logger;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/
/******************************************************************************
 * Method: Constructor
 * Description: Default constructor.  Is likely never called, but wanted to
 *              define it explicitly.
 ******************************************************************************/
Packet::Packet() {
    m_tPacketType = UNKNOWN;
    m_iPacketSize = 0;
    m_pPacket = NULL;
}

/******************************************************************************
 * Method: Destructor
 * Description: free up our dynamically created packet data.
 ******************************************************************************/
Packet::~Packet() {
	LOG(DEBUG) << "Packet DTOR";
    if( m_pPacket ) {
        delete [] m_pPacket;
        m_pPacket = NULL;
    }
	LOG(DEBUG) << "Packet DTOR exit";
}

/******************************************************************************
 * Method: asAscii
 * Description: an ascii representation of the packet.
 ******************************************************************************/
string Packet::asAscii() {
    ostringstream out;

    char* packetBuffer = packet();

    out << "<" << asciiPacketLabel() << " type=\"" << asciiPacketType() << "\" "
        << "time=\"" << asciiPacketTimestamp() << "\">";

    if(packetBuffer) {
        for(int i = HEADER_SIZE; i < packetSize(); i++)
            out << packetBuffer[i];
    }

    out << "</" << asciiPacketLabel() << ">\n\r";

    return out.str();
}

/******************************************************************************
 *   PRIVATE METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: typeToString
 * Description: Convert a packet type to a string representation.
 *
 * Parameters:
 *   type - enum element packet type
 *
 * Return:
 *   string representation of that type.
 *
 ******************************************************************************/
string Packet::typeToString(PacketType type) {
    switch(type) {
        case UNKNOWN: return string("UNKNOWN");
        case DATA_FROM_INSTRUMENT: return string("DATA_FROM_INSTRUMENT");
        case DATA_FROM_DRIVER: return string("DATA_FROM_DRIVER");
        case PORT_AGENT_COMMAND: return string("PORT_AGENT_COMMAND");
        case PORT_AGENT_STATUS: return string("PORT_AGENT_STATUS");
        case PORT_AGENT_FAULT: return string("PORT_AGENT_FAULT");
        case INSTRUMENT_COMMAND: return string("INSTRUMENT_COMMAND");
        case PORT_AGENT_HEARTBEAT: return string("PORT_AGENT_HEARTBEAT");
    };

    return "OUT_OF_RANGE";
}


