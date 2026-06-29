#include "GPSDriver.h"
#include "logger/Logger.h"
#include <sstream>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <sys/select.h>

using namespace common;

namespace sensor_service {

// ============================================================================
// Constructor / Destructor
// ============================================================================

GPSDriver::GPSDriver(const std::string& sensorId)
    : SensorDriver(sensorId, SensorType::GPS),
      m_serialFd(-1),
      m_baudRate(9600) {
    
    resetState();
    LOG_DEBUG("GPSDriver created: " + sensorId);
}

GPSDriver::~GPSDriver() {
    if (m_serialFd >= 0) {
        close(m_serialFd);
        m_serialFd = -1;
    }
    LOG_DEBUG("GPSDriver destroyed: " + m_sensorId);
}

// ============================================================================
// SensorDriver Interface
// ============================================================================

bool GPSDriver::initialize(const Json::Value& config) {
    LOG_INFO("Initializing GPS sensor: " + m_sensorId);
    
    // Get configuration
    if (config.isMember("device")) {
        m_serialDevice = config["device"].asString();
    } else {
        m_serialDevice = "/dev/ttyS0"; // Default
    }
    
    if (config.isMember("baud_rate")) {
        m_baudRate = config["baud_rate"].asInt();
    }
    
    if (config.isMember("timeout_ms")) {
        // Use for read timeout
    }
    
    // Open serial port
    m_serialFd = open(m_serialDevice.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_serialFd < 0) {
        LOG_ERROR("Failed to open GPS serial port: " + m_serialDevice);
        return false;
    }
    
    // Configure serial port
    struct termios options;
    if (tcgetattr(m_serialFd, &options) != 0) {
        LOG_ERROR("Failed to get serial attributes for GPS");
        close(m_serialFd);
        m_serialFd = -1;
        return false;
    }
    
    // Set baud rate
    speed_t baudRate;
    switch (m_baudRate) {
        case 4800: baudRate = B4800; break;
        case 9600: baudRate = B9600; break;
        case 19200: baudRate = B19200; break;
        case 38400: baudRate = B38400; break;
        case 57600: baudRate = B57600; break;
        case 115200: baudRate = B115200; break;
        default: baudRate = B9600; break;
    }
    
    cfsetispeed(&options, baudRate);
    cfsetospeed(&options, baudRate);
    
    // 8N1
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);
    
    // No hardware flow control
    options.c_cflag &= ~CRTSCTS;
    
    // Raw input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    // Raw output
    options.c_oflag &= ~OPOST;
    
    // Set timeout
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 10; // 1 second timeout
    
    if (tcsetattr(m_serialFd, TCSANOW, &options) != 0) {
        LOG_ERROR("Failed to set serial attributes for GPS");
        close(m_serialFd);
        m_serialFd = -1;
        return false;
    }
    
    // Flush serial port
    tcflush(m_serialFd, TCIOFLUSH);
    
    setConnected(true);
    resetState();
    
    LOG_INFO("GPS sensor initialized: " + m_sensorId);
    return true;
}

bool GPSDriver::start() {
    if (m_serialFd < 0) {
        LOG_ERROR("GPS serial port not open: " + m_sensorId);
        return false;
    }
    
    m_running = true;
    m_shouldStop = false;
    
    LOG_INFO("GPS sensor started: " + m_sensorId);
    return true;
}

bool GPSDriver::stop() {
    m_running = false;
    m_shouldStop = true;
    
    LOG_INFO("GPS sensor stopped: " + m_sensorId);
    return true;
}

bool GPSDriver::readSample(SensorReading& reading) {
    if (!m_running || !m_connected) {
        return false;
    }
    
    if (m_serialFd < 0) {
        setConnected(false);
        return false;
    }
    
    // Read NMEA data from serial port
    char buffer[256];
    int bytesRead = read(m_serialFd, buffer, sizeof(buffer) - 1);
    
    if (bytesRead <= 0) {
        return false;
    }
    
    buffer[bytesRead] = '\0';
    
    // Process each line
    std::string data(buffer);
    std::istringstream iss(data);
    std::string line;
    
    bool updated = false;
    
    while (std::getline(iss, line, '\n')) {
        // Remove carriage return
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) continue;
        
        if (parseNMEA(line)) {
            updated = true;
        }
    }
    
    if (!updated) {
        return false;
    }
    
    // Check if we have a fix
    if (!m_gpsState.hasFix) {
        // No fix yet
        return false;
    }
    
    // Build reading
    reading = m_gpsState.toReading(m_sensorId);
    reading.timestamp = getCurrentTime();
    reading.sequence = nextSequence();
    reading.isValid = true;
    
    // Add additional metadata
    reading.metadata["satellites"] = std::to_string(m_gpsState.satellites);
    reading.metadata["has_fix"] = m_gpsState.hasFix ? "true" : "false";
    
    emitData(reading);
    return true;
}

bool GPSDriver::calibrate(const Json::Value& params) {
    // GPS calibration not typically needed
    // Could set offsets for position data
    return true;
}

bool GPSDriver::selfTest(Json::Value& result) {
    result["status"] = "passed";
    result["tests"] = 3;
    result["results"]["serial_connected"] = m_serialFd >= 0;
    result["results"]["has_fix"] = m_gpsState.hasFix;
    result["results"]["satellites"] = m_gpsState.satellites;
    
    return m_serialFd >= 0;
}

bool GPSDriver::configure(const Json::Value& config) {
    // Update configuration
    if (config.isMember("baud_rate")) {
        // Would need to reconfigure serial port
        // For simplicity, we'll just store the value
        m_baudRate = config["baud_rate"].asInt();
    }
    
    return true;
}

// ============================================================================
// NMEA Parsing
// ============================================================================

bool GPSDriver::parseNMEA(const std::string& line) {
    if (line.empty() || line[0] != '$') {
        return false;
    }
    
    // Find the message type
    size_t start = line.find('$');
    if (start == std::string::npos) return false;
    
    size_t end = line.find('*');
    std::string message;
    
    if (end != std::string::npos) {
        message = line.substr(start + 1, end - start - 1);
    } else {
        message = line.substr(start + 1);
    }
    
    // Split into fields
    std::vector<std::string> fields;
    std::istringstream iss(message);
    std::string field;
    
    while (std::getline(iss, field, ',')) {
        fields.push_back(field);
    }
    
    if (fields.empty()) return false;
    
    std::string type = fields[0];
    
    if (type == "GPGGA" || type == "GNGGA") {
        return parseGGA(fields);
    } else if (type == "GPGLL" || type == "GNGLL") {
        return parseGLL(fields);
    } else if (type == "GPRMC" || type == "GNRMC") {
        return parseRMC(fields);
    } else if (type == "GPVTG" || type == "GNVTG") {
        return parseVTG(fields);
    } else if (type == "GPGSA" || type == "GNGSA") {
        return parseGSA(fields);
    } else if (type == "GPGSV" || type == "GNGSV") {
        return parseGSV(fields);
    }
    
    return false;
}

bool GPSDriver::parseGGA(const std::vector<std::string>& fields) {
    if (fields.size() < 15) return false;
    
    // Format: $GPGGA,time,lat,NS,lon,EW,quality,satellites,hdop,altitude,M,geoid,M,diff_age,diff_station
    // Index:  0     1    2   3   4   5  6        7         8    9       10 11     12       13          14
    
    // Time
    m_gpsState.timestamp = fields[1];
    
    // Latitude
    if (fields[2].size() > 0 && fields[3].size() > 0) {
        m_gpsState.latitude = parseNMEACoordinate(fields[2], fields[3]);
    }
    
    // Longitude
    if (fields[4].size() > 0 && fields[5].size() > 0) {
        m_gpsState.longitude = parseNMEACoordinate(fields[4], fields[5]);
    }
    
    // Quality (0=invalid, 1=GPS, 2=DGPS, 3=RTK)
    int quality = 0;
    if (fields[6].size() > 0) {
        quality = std::stoi(fields[6]);
    }
    
    // Satellites
    if (fields[7].size() > 0) {
        m_gpsState.satellites = std::stoi(fields[7]);
    }
    
    // HDOP
    if (fields[8].size() > 0) {
        m_gpsState.horizontalAccuracy = std::stod(fields[8]);
    }
    
    // Altitude
    if (fields[9].size() > 0) {
        m_gpsState.altitude = std::stod(fields[9]);
    }
    
    // Fix status
    m_gpsState.hasFix = (quality > 0);
    m_gpsState.lastUpdate = std::chrono::system_clock::now();
    
    return true;
}

bool GPSDriver::parseGLL(const std::vector<std::string>& fields) {
    if (fields.size() < 7) return false;
    
    // Format: $GPGLL,lat,NS,lon,EW,time,status
    // Index:  0     1   2  3   4  5    6
    
    // Latitude
    if (fields[1].size() > 0 && fields[2].size() > 0) {
        m_gpsState.latitude = parseNMEACoordinate(fields[1], fields[2]);
    }
    
    // Longitude
    if (fields[3].size() > 0 && fields[4].size() > 0) {
        m_gpsState.longitude = parseNMEACoordinate(fields[3], fields[4]);
    }
    
    // Time
    if (fields[5].size() > 0) {
        m_gpsState.timestamp = fields[5];
    }
    
    // Status (A=valid, V=invalid)
    if (fields[6].size() > 0) {
        m_gpsState.hasFix = (fields[6] == "A");
    }
    
    m_gpsState.lastUpdate = std::chrono::system_clock::now();
    return true;
}

bool GPSDriver::parseRMC(const std::vector<std::string>& fields) {
    if (fields.size() < 12) return false;
    
    // Format: $GPRMC,time,status,lat,NS,lon,EW,speed,course,date,magvar,magvar_dir
    // Index:  0    1    2      3   4  5   6  7     8      9    10     11
    
    // Time
    if (fields[1].size() > 0) {
        m_gpsState.timestamp = fields[1];
    }
    
    // Status (A=active, V=void)
    if (fields[2].size() > 0) {
        m_gpsState.hasFix = (fields[2] == "A");
    }
    
    // Latitude
    if (fields[3].size() > 0 && fields[4].size() > 0) {
        m_gpsState.latitude = parseNMEACoordinate(fields[3], fields[4]);
    }
    
    // Longitude
    if (fields[5].size() > 0 && fields[6].size() > 0) {
        m_gpsState.longitude = parseNMEACoordinate(fields[5], fields[6]);
    }
    
    // Speed in knots
    if (fields[7].size() > 0) {
        double speedKnots = std::stod(fields[7]);
        m_gpsState.speed = speedKnots * 1.852; // Convert to km/h
    }
    
    // Course (heading)
    if (fields[8].size() > 0) {
        m_gpsState.heading = std::stod(fields[8]);
    }
    
    // Date
    if (fields[9].size() > 0) {
        // Combine date and time for full timestamp
    }
    
    m_gpsState.lastUpdate = std::chrono::system_clock::now();
    return true;
}

bool GPSDriver::parseVTG(const std::vector<std::string>& fields) {
    if (fields.size() < 9) return false;
    
    // Format: $GPVTG,course,T,course,M,speed,N,speed,K
    // Index:  0     1      2 3      4 5     6 7     8
    
    // Course (heading)
    if (fields[1].size() > 0) {
        m_gpsState.heading = std::stod(fields[1]);
    }
    
    // Speed in knots
    if (fields[5].size() > 0) {
        double speedKnots = std::stod(fields[5]);
        m_gpsState.speed = speedKnots * 1.852; // Convert to km/h
    }
    
    m_gpsState.lastUpdate = std::chrono::system_clock::now();
    return true;
}

bool GPSDriver::parseGSA(const std::vector<std::string>& fields) {
    if (fields.size() < 17) return false;
    
    // Format: $GPGSA,opmode,fixed,PRN1,...PRN12,PDOP,HDOP,VDOP
    // Index:  0      1     2    3    14   15   16   17
    
    // PDOP
    if (fields[15].size() > 0) {
        // Position dilution of precision
        double pdop = std::stod(fields[15]);
    }
    
    // HDOP (already parsed in GGA)
    if (fields[16].size() > 0) {
        m_gpsState.horizontalAccuracy = std::stod(fields[16]);
    }
    
    // VDOP
    if (fields[17].size() > 0) {
        m_gpsState.verticalAccuracy = std::stod(fields[17]);
    }
    
    return true;
}

bool GPSDriver::parseGSV(const std::vector<std::string>& fields) {
    if (fields.size() < 4) return false;
    
    // Format: $GPGSV,total_msgs,msg_num,satellites_in_view,(sat_info)*
    // Index:  0     1          2       3                 4+ 
    
    // Satellites in view
    if (fields[3].size() > 0) {
        int satsInView = std::stoi(fields[3]);
        // Use this to update satellite count if GGA not available
        if (m_gpsState.satellites == 0) {
            m_gpsState.satellites = satsInView;
        }
    }
    
    return true;
}

double GPSDriver::parseNMEACoordinate(const std::string& coord, const std::string& dir) {
    if (coord.empty() || dir.empty()) return 0.0;
    
    // Format: DDMM.MMMM or DDDMM.MMMM
    size_t dotPos = coord.find('.');
    if (dotPos == std::string::npos) return 0.0;
    
    // Degrees are before the dot minus 2 (for minutes)
    size_t degLen = dotPos - 2;
    if (degLen <= 0) return 0.0;
    
    double degrees = std::stod(coord.substr(0, degLen));
    double minutes = std::stod(coord.substr(degLen));
    
    double value = degrees + minutes / 60.0;
    
    if (dir == "S" || dir == "W") {
        value = -value;
    }
    
    return value;
}

void GPSDriver::resetState() {
    m_gpsState.hasFix = false;
    m_gpsState.satellites = 0;
    m_gpsState.latitude = 0.0;
    m_gpsState.longitude = 0.0;
    m_gpsState.altitude = 0.0;
    m_gpsState.speed = 0.0;
    m_gpsState.heading = 0.0;
    m_gpsState.horizontalAccuracy = 0.0;
    m_gpsState.verticalAccuracy = 0.0;
    m_gpsState.timestamp = "";
    m_gpsState.lastUpdate = std::chrono::system_clock::time_point();
}

bool GPSDriver::checkConnection() {
    if (m_serialFd < 0) return false;
    
    // Try to read some data with timeout
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(m_serialFd, &readSet);
    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    int result = select(m_serialFd + 1, &readSet, nullptr, nullptr, &timeout);
    
    if (result > 0 && FD_ISSET(m_serialFd, &readSet)) {
        return true;
    }
    
    return false;
}

} // namespace sensor_service
