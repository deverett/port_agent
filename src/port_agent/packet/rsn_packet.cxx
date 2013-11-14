/*******************************************************************************
 * Class: RSNPacket
 * Filename: rsn_packet.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Basic packet object for the port agent.  Packets are teh communication
 * protocol for the port agent to outside interfaces via the data and command
 * port.  They are transmitted as binary datagrams with the following structure.
 *
 * This is the most basic type of packet.  Data is given in the constructor
 * and the packet is created.  Once created there is no need to modify the
 * packet and it should be sent immediatly.
 * 
 * A packet contains:
 *
 * sync series      24 bits
 * message type     8 bits
 * packet size      16 bits (including the header)
 * checksum         16 bits
 * timestamp        64 bits
 * payload          variable size
 *
 * Usage:
 *
 * RSNPacket packet(DATA_FROM_DRIVER, timestamp, payload, length);
 *
 * if(packet.readyToSend())
 *    write(packet.packet(), packet().packetSize());
 *    
 ******************************************************************************/

#include "rsn_packet.h"
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
RSNPacket::RSNPacket() {
    m_tPacketType = UNKNOWN;
    m_iPacketSize = 0;
    //m_iChecksum = 0;
    m_pPacket = NULL;
}

/******************************************************************************
 * Method: Constructor
 * Description: General use constructor for the the packet.  Sets up all
 *              paramaters, basically an immutable object.
 * Parameters:
 *   packetType - type of packet.  See the PacketTypeEnum
 *   timestamp  - Timestamp when the data was initially collected.
 *   payload - the actual data stored in the packet.
 *   payloadSize - number of bytes in the payload.
 *   sentinleSequenceSize - Size of the sentinle sequences.  This is needed
 *                          because we don't know the size of the
 *                          sentinleSequence string.  We can't use a null
 *                          teminated string because \0 may be part of the
 *                          sequence.
 * Throws:
 *
 * PacketParameterOutOfRange - raised when
 *     - packet type is UNKNOWN
 *
 ******************************************************************************/
RSNPacket::RSNPacket(PacketType packetType, char *pPacket, uint16_t iPacketSize) {
    
    LOG(DEBUG) << "Building a new RSNPacket";

    if (0 == packetType) {
        throw PacketParamOutOfRange("invalid packet type");
    }
    

    /*
     * DHE TODO: Not sure we want to copy data here but this is how the other
     * packet objects work.
     */
    m_tPacketType = packetType;
    m_iPacketSize = iPacketSize;
    m_pPacket = new char[m_iPacketSize];
    
    LOG(DEBUG1) << "Setting packet header info";
    
    if (pPacket) {
        LOG(DEBUG1) << "Deep copy packet, size: " << m_iPacketSize;
        // Deep copy the data
        for(int i = 0; i < m_iPacketSize; i++)
            m_pPacket[i] = pPacket[i];
    }
    
    LOG(DEBUG1) << "Deep copy complete";
    
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy of the packet data.
 *
 * Parameters:
 *   copy - rhs object to copy
 *
 ******************************************************************************/
RSNPacket::RSNPacket(const RSNPacket& rhs) {
    LOG(DEBUG) << "RSNPacket copy constructor";
    
    m_pPacket = NULL;
    copy(rhs);
}

/******************************************************************************
 * Method: Destructor
 * Description: free up our dynamically created packet data.
 ******************************************************************************/
RSNPacket::~RSNPacket() {
	LOG(DEBUG) << "RSNPacket DTOR";
    if( m_pPacket ) {
        delete [] m_pPacket;
        m_pPacket = NULL;
    }
	LOG(DEBUG) << "RSNPacket DTOR exit";
}

/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor ensuring we do a deep copy of the packet data.
 *
 * Parameters:
 *   copy - rhs object to copy
 *
 ******************************************************************************/
RSNPacket & RSNPacket::operator=(const RSNPacket &rhs) {

	if(m_pPacket) {
		delete m_pPacket;
		m_pPacket = NULL;
	}

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
void RSNPacket::copy(const RSNPacket &copy) {
    m_tPacketType = copy.m_tPacketType;
    m_iPacketSize = copy.m_iPacketSize;

    // Deep copy the payload
    if(copy.m_pPacket) {
        m_pPacket = new char[packetSize()];
        for(int i = 0; i < packetSize(); i++)
            m_pPacket[i] = copy.m_pPacket[i];
    } else {
    	m_pPacket = NULL;
    }
}


/******************************************************************************
 * Method: asAscii
 * Description: an ascii representation of the packet.
 ******************************************************************************/
string RSNPacket::asAscii() {
    ostringstream out;

    char* packetBuffer = packet();

    out << "<" << asciiPacketLabel() << " type=\"" << asciiPacketType() << "\">";

    if(packetBuffer) {
        for(int i = HEADER_SIZE; i < packetSize(); i++)
            out << packetBuffer[i];
    }

    out << "</" << asciiPacketLabel() << ">\n\r";

    return out.str();
}

/******************************************************************************
 * Method: pretty
 * Description: return a human readable ascii formatted string. Useful for
 *              logging.
 ******************************************************************************/
string RSNPacket::pretty() {
    ostringstream out;
    
    char* packetBuffer = packet();
	
    // Output some pretty header information from the packet.  Ready to send
    // is not part of the packet header, but it sure is useful information.
    out << endl;
    out << "Ready to send: ";
    if(readyToSend())
        out << "true" << endl;
    else
        out << "false" << endl;
    out << "Sync: " << "0x" << hex << SYNC << dec << endl;
    out << "Type: " << m_tPacketType << " (" << typeToString(m_tPacketType) << ")" << endl;
    out << "Size: " << m_iPacketSize << endl;
    //out << "Checksum: " << hex << m_iChecksum << dec << endl;
    //out << "Timestamp: " << m_oTimestamp.asNumber() << endl;
	
	LOG(DEBUG) << "Size: " << m_iPacketSize;
    
    // Let's dump some raw data, first as ascii.
    out << "Payload (ascii): ";
    if (packetBuffer) {
        out << endl;
        for(int i = HEADER_SIZE; i < packetSize(); i++)
            if(isprint(packetBuffer[i]))
                out << packetBuffer[i];
            else
                out << "0x" << hex << byteToUnsignedInt(packetBuffer[i]);
    } else {
        out << "<NULL>" << endl;
    }
    out << endl;
	LOG(DEBUG) << "Completed acsii generation";

    // Hex out, packet data
    out << "Payload (hex): ";
    if(packetBuffer) {
        for(int i = HEADER_SIZE; i < packetSize(); i++) {
            // Wrap lines
            if(i % 16 == 0)
                out << endl;
            out << hex << byteToUnsignedInt(packetBuffer[i]) << " ";
        }
    } else {
        out << "<NULL>" << endl;
    }
    out << endl;
	LOG(DEBUG) << "Completed hex generation";
    
    // Finally the full packet hex output.  Shows the header.
    out << "Full RSNPacket (hex): ";
    if(packetBuffer) {
        for(int i = 0; i < packetSize(); i++) {
            // Wrap lines
            if(i % 16 == 0)
                out << endl;
            out << hex << setw(2) << byteToUnsignedInt(packetBuffer[i]) << " ";
        }
    } else {
        out << "<NULL>" << endl;
    }
	LOG(DEBUG) << "Completed hex packet generation";
    
    return out.str();
}

/******************************************************************************
 * Method: packet
 * Description: Reconstruct the packet header in the buffer.
 *
 * Make sure we have converted everything to big-endian!
 *
 * Return:
 *   copy - return a pointer to the actual data buffer.  Note that this is only
 *          a pointer so if this object goes out of scope the buffer will be
 *          destroyed.
 ******************************************************************************/
char* RSNPacket::packet() {
    
    return m_pPacket;
}


/******************************************************************************
 *   PRIVATE METHODS
 ******************************************************************************/

