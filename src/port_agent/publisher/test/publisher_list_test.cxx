#include "publisher_test.h"
#include "common/logger.h"
#include "common/log_file.h"
#include "common/util.h"
#include "port_agent/packet/port_agent_packet.h"
#include "port_agent/publisher/publisher_list.h"
#include "port_agent/publisher/driver_command_publisher.h"
#include "port_agent/publisher/instrument_command_publisher.h"
#include "port_agent/publisher/tcp_publisher.h"
#include "port_agent/publisher/udp_publisher.h"
#include "port_agent/publisher/log_publisher.h"

#include "network/udp_comm_socket.h"
#include "network/tcp_comm_socket.h"
#include "network/tcp_comm_listener.h"

#include "gtest/gtest.h"

#include <sstream>
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace logger;
using namespace publisher;
using namespace packet;

const int OBSERVATORY_COMMAND_PORT = 5001;
const int OBSERVATORY_DATA_PORT    = 5002;
const int INSTRUMENT_DATA_PORT     = 5003;

class PublisherListTest : public testing::Test {
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "   PublisherList Test Start Up";
            LOG(INFO) << "************************************************";
        }
};

TEST_F(PublisherListTest, DISABLED_CTOR) {
	PublisherList list;
    
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(OBSERVATORY_COMMAND_PORT);
	
	TCPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(OBSERVATORY_DATA_PORT);
	
    InstrumentCommandPublisher publisherA(&socketA);
    InstrumentCommandPublisher publisherB(&socketB);
	TCPCommSocket *comm;
	InstrumentCommandPublisher *pub;
	
	list.add(&publisherA);
	pub = ((InstrumentCommandPublisher *)(list.front()));
	comm = (TCPCommSocket *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), OBSERVATORY_COMMAND_PORT);
	
	list.add(&publisherB);
	pub = ((InstrumentCommandPublisher *)(list.front()));
	comm = (TCPCommSocket *)(pub->commSocket());
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), OBSERVATORY_DATA_PORT);
}

TEST_F(PublisherListTest, DISABLED_CommListener) {
	try {
		
	PublisherList list;
    
	LOG(DEBUG) << "Initialize Socket A";
	TCPCommListener socketA;
	socketA.setPort(OBSERVATORY_COMMAND_PORT);
	EXPECT_EQ(socketA.port(), OBSERVATORY_COMMAND_PORT);
	socketA.initialize();
	EXPECT_EQ(socketA.getListenPort(), OBSERVATORY_COMMAND_PORT);
	
	LOG(DEBUG) << "Initialize Socket B";
	TCPCommListener socketB;
    socketB.setPort(OBSERVATORY_DATA_PORT);
	socketB.initialize();
	
	LOG(DEBUG) << "Initialize Publishers";
    DriverCommandPublisher publisherA(&socketA);
    DriverCommandPublisher publisherB(&socketB);
	TCPCommListener *comm;
	DriverCommandPublisher *pub;
	
	list.add(&publisherA);
	pub = ((DriverCommandPublisher *)(list.front()));
	comm = (TCPCommListener *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->getListenPort(), OBSERVATORY_COMMAND_PORT);
	
	list.add(&publisherB);
	pub = ((DriverCommandPublisher *)(list.front()));
	comm = (TCPCommListener *)(pub->commSocket());
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->getListenPort(), OBSERVATORY_DATA_PORT);
	}
	catch(OOIException &e) {
		string msg = e.what();
		LOG(ERROR) << "Exception: " << msg;
		ASSERT_FALSE(true);
	};
}


TEST_F(PublisherListTest, DISABLED_UDPSocket) {
	PublisherList list;
    
	UDPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(OBSERVATORY_COMMAND_PORT);
	
	UDPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(OBSERVATORY_DATA_PORT);
	
    UDPPublisher publisherA(&socketA);
    UDPPublisher publisherB(&socketB);
	UDPCommSocket *comm;
	UDPPublisher *pub;
	
	list.add(&publisherA);
	pub = ((UDPPublisher *)(list.back()));
	comm = (UDPCommSocket *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), OBSERVATORY_COMMAND_PORT);
	
	list.add(&publisherB);
	pub = ((UDPPublisher *)(list.back()));
	comm = (UDPCommSocket *)(pub->commSocket());
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(comm->port(), OBSERVATORY_DATA_PORT);
}

TEST_F(PublisherListTest, DISABLED_TCPSocket) {
	PublisherList list;
    
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(OBSERVATORY_COMMAND_PORT);
	
	TCPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(OBSERVATORY_DATA_PORT);
	
    TCPPublisher publisherA(&socketA);
    TCPPublisher publisherB(&socketB);
	TCPCommSocket *comm;
	TCPPublisher *pub;
	
	list.add(&publisherA);
	pub = ((TCPPublisher *)(list.back()));
	comm = (TCPCommSocket *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), OBSERVATORY_COMMAND_PORT);
	
	list.add(&publisherB);
	pub = ((TCPPublisher *)(list.back()));
	comm = (TCPCommSocket *)(pub->commSocket());
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(comm->port(), OBSERVATORY_DATA_PORT);
}

TEST_F(PublisherListTest, DISABLED_File) {
    LogPublisher publisher;
	PublisherList list;
    
	list.add(&publisher);
	EXPECT_EQ(list.size(), 1);
}

TEST_F(PublisherListTest, DISABLED_PublishTest) {
	PublisherList list;
	LogPublisher publisher;
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(OBSERVATORY_COMMAND_PORT);
    TCPPublisher publisherA(&socketA);
            
	Timestamp ts(1, 0x80000000);
    PortAgentPacket packet(DATA_FROM_DRIVER, ts, "data", 4);
	
	list.add(&publisher);
	EXPECT_EQ(list.size(), 1);
	
	list.add(&publisherA);
	EXPECT_EQ(list.size(), 2);
	
	list.publish(&packet);
}

TEST_F(PublisherListTest, DISABLED_DuplicateAddTest) {
	PublisherList list;
	LogPublisher publisher;
	
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(OBSERVATORY_COMMAND_PORT);
    TCPPublisher publisherA(&socketA);
            
	TCPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(OBSERVATORY_COMMAND_PORT);
    TCPPublisher publisherB(&socketB);
            
	list.add(&publisher);
	EXPECT_EQ(list.size(), 1);
	
	list.add(&publisherA);
	EXPECT_EQ(list.size(), 2);
	
	list.add(&publisherB);
	EXPECT_EQ(list.size(), 2);
}

TEST_F(PublisherListTest, SearchAndSetRotation) {
	PublisherList list;
	LogPublisher publisher;
	
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(OBSERVATORY_COMMAND_PORT);
    TCPPublisher publisherA(&socketA);
            
	list.add(&publisherA);
	EXPECT_EQ(list.size(), 1);
	
	EXPECT_FALSE(list.searchByType(PUBLISHER_FILE));
	
	list.add(&publisher);
	EXPECT_EQ(list.size(), 2);
	Publisher *found = list.searchByType(PUBLISHER_FILE);
	EXPECT_TRUE(found);
	
	((FilePublisher*)found)->setRotationInterval(HOURLY);
}
