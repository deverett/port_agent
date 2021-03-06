/*******************************************************************************
 * Class: DriverPublisher
 * Filename: driver_publisher.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Base class for publishers writing to driver connections.
 *
 ******************************************************************************/

#include "driver_publisher.h"
#include "common/logger.h"
#include "common/exception.h"
#include "port_agent/packet/packet.h"

#include <sstream>
#include <string>

#include <stdio.h>

using namespace std;
using namespace packet;
using namespace logger;
using namespace publisher;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/


/******************************************************************************
 * Method: Constructor
 * Description: default constructor
 ******************************************************************************/
DriverPublisher::DriverPublisher() : FilePointerPublisher() { }

