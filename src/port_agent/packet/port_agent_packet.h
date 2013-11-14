/*******************************************************************************
 * Class: PortAgentPacket
 * Filename: packet.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Basic packet object for the port agent.  Packets are teh communication
 * protocol for the port agent to outside interfaces via the data and command
 * port.  They are transmitted as binary datagrams with the following structure.
 *
 * This is the most basic type of packet.  Data is given in the constructor
 * and the packet is created.  Once created there is no need to modify the
 * packet and it should be sent immediately.
 *
 * NOTE: This packet will likely never directly be used in code, but extended
 * to handle different input methods.  That said, it could be used if we know
 * the entire content of the packet before it is created.
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
 * PortAgentPacket packet(DATA_FROM_DRIVER, timestamp, payload, length);
 *
 * if(packet.readyToSend())
 *    write(packet.packet(), packet().packetSize());
 *    
 ******************************************************************************/

#ifndef __PORT_AGENT_PACKET_H_
#define __PORT_AGENT_PACKET_H_

#include "common/timestamp.h"

#include <string>
#include <stdint.h>

#include "packet.h"

using namespace packet;
using namespace std;

class PortAgentPacket : public Packet {
    /********************
     *      METHODS     *
     ********************/

    public:
        ///////////////////////
        // Public Methods
        PortAgentPacket();
        PortAgentPacket(PacketType packet_type, Timestamp timestamp,
               char *payload, uint16_t payload_size);
        PortAgentPacket(const PortAgentPacket &rhs);
        virtual ~PortAgentPacket();
        
        /* Operators */
        virtual PortAgentPacket & operator=(const PortAgentPacket &rhs);

        /* Accessors */
        PacketType packetType() { return m_tPacketType; }
        uint16_t packetSize()    { return m_iPacketSize; }
        uint16_t payloadSize()   { return m_iPacketSize - HEADER_SIZE; }
        uint16_t checksum()      { return m_iChecksum; }
        Timestamp timestamp()    { return m_oTimestamp; }
        char* payload()          { return m_pPacket + HEADER_SIZE; }
        char* packet();

        // return a ASCII string representation of the packet
        virtual string asAscii();

        // return a pretty string representation of the packet
        string pretty();

        // The basic packets are always ready to send.  This will need to be
        // overloaded for buffered packets.
        virtual bool readyToSend() { return true; }

        // Convert a PacketType to a string representation
        string typeToString(PacketType type);
    protected:

        // Calculate a checksum of the packet buffer.
        virtual uint16_t calculateChecksum();

        // deep copy a packet object
        virtual void copy(const PortAgentPacket &copy);

        // ascii packet label
        string asciiPacketLabel() { return "port_agent_packet"; }
        string asciiPacketTimestamp() { return m_oTimestamp.asNumber(); }
        string asciiPacketType() { return typeToString(m_tPacketType); }


    private:

    /********************
     *      MEMBERS     *
     ********************/

    protected:
        
        PacketType m_tPacketType;
        uint16_t m_iPacketSize;
        uint16_t m_iChecksum;
        Timestamp m_oTimestamp;
        char *m_pPacket;

};

#endif //__PORT_AGENT_PACKET_H_
