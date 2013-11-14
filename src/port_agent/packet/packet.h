/*******************************************************************************
 * Class: Packet
 * Filename: packet.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Basic packet object for the port agent.  This base class is necessary because
 * RSN DIGI packets arrive with 16-byte headers, but we still need a common packet
 * type that both RSN DIGI packets and port agent packets use.  The reason is that
 * all of the publishers want a packet object.  Packets are the communication
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
 ******************************************************************************/

#ifndef __PACKET_H_
#define __PACKET_H_

#include "common/timestamp.h"

#include <string>
#include <stdint.h>

using namespace std;

namespace packet {
    
    /* Known packet types */
    enum PacketType {
        UNKNOWN,  
        DATA_FROM_INSTRUMENT,
        DATA_FROM_RSN,
        DATA_FROM_DRIVER,
        PORT_AGENT_COMMAND,
        PORT_AGENT_STATUS,
        PORT_AGENT_FAULT,
        INSTRUMENT_COMMAND,
        PORT_AGENT_HEARTBEAT
    };

    const uint32_t SYNC = 0xA39D7A;
    const short    HEADER_SIZE = 16;


    class Packet {
        /********************
         *      METHODS     *
         ********************/
        
        public:
            ///////////////////////
            // Public Methods
            Packet();
            virtual ~Packet();
            
            /* Accessors */
            PacketType packetType() { return m_tPacketType; }
            uint16_t packetSize()    { return m_iPacketSize; }
            uint16_t payloadSize()   { return m_iPacketSize - HEADER_SIZE; }
            char* payload()          { return m_pPacket + HEADER_SIZE; }
            virtual char* packet() = 0;  // must be implemented by subclasses
            
            // return a ASCII string representation of the packet
            string asAscii();

            // return a pretty string representation of the packet
            virtual string pretty() = 0;
            
            // The basic packets are always ready to send.  This will need to be
            // overloaded for buffered packets.
            virtual bool readyToSend() { return true; }

            // Convert a PacketType to a string representation
            string typeToString(PacketType type);
        protected:

             string asciiPacketLabel() { return "packet"; }
            virtual string asciiPacketTimestamp() = 0; // must be implemented by subclasses.
            string asciiPacketType() { return typeToString(m_tPacketType); }


        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
            
            PacketType m_tPacketType;
            uint16_t m_iPacketSize;
            char *m_pPacket;

    };
}

#endif //__PACKET_H_
