markdown

# BBB AI-64 API Reference

## Overview

This document provides a comprehensive API reference for the BBB AI-64 platform libraries and services.

## Core Libraries

### Device Manager API

#### DeviceManager Class

```cpp
class DeviceManager {
public:
    // Singleton access
    static DeviceManager& getInstance();
    
    // Device management
    bool addDevice(std::unique_ptr<Device> device);
    bool removeDevice(const std::string& deviceId);
    Device* getDevice(const std::string& deviceId);
    std::vector<Device*> getAllDevices();
    std::vector<Device*> getDevicesByType(DeviceType type);
    
    // Device operations
    bool enableDevice(const std::string& deviceId);
    bool disableDevice(const std::string& deviceId);
    bool resetDevice(const std::string& deviceId);
    bool calibrateDevice(const std::string& deviceId, const Json::Value& params);
    bool selfTestDevice(const std::string& deviceId, Json::Value& result);
    bool configureDevice(const std::string& deviceId, const Json::Value& config);
    
    // Data operations
    bool readData(const std::string& deviceId, DeviceSample& data, int timeoutMs = 5000);
    bool writeData(const std::string& deviceId, const Json::Value& data, int timeoutMs = 5000);
    
    // Events
    void setEventCallback(EventCallback callback);
    void setCommandCallback(CommandCallback callback);
    
    // Configuration
    bool saveConfig(const std::string& path) const;
    bool loadConfig(const std::string& path);
};

Device Class
cpp

class Device {
public:
    // Lifecycle
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool reset() = 0;
    
    // Data operations
    virtual bool readData(DeviceSample& data, int timeoutMs = 5000) = 0;
    virtual bool writeData(const Json::Value& data, int timeoutMs = 5000) = 0;
    virtual bool executeCommand(const DeviceCommand& command, DeviceSample& result, int timeoutMs = 5000) = 0;
    
    // Calibration
    virtual bool calibrate(const Json::Value& params, Json::Value& result, int timeoutMs = 5000) = 0;
    virtual bool selfTest(Json::Value& result, int timeoutMs = 5000) = 0;
    virtual bool configure(const Json::Value& config, int timeoutMs = 5000) = 0;
    
    // Information
    virtual DeviceInfo getInfo() const;
    virtual DeviceStatus getStatus() const;
    virtual DeviceCapabilities getCapabilities() const;
    virtual std::string getId() const;
    virtual std::string getName() const;
    virtual DeviceType getType() const;
    
    // Callbacks
    virtual void setDataCallback(DataCallback callback);
    virtual void setEventCallback(EventCallback callback);
    virtual void setCommandCallback(CommandCallback callback);
};

Device Types
cpp

enum class DeviceType {
    SENSOR_IMU,
    SENSOR_GPS,
    SENSOR_TEMPERATURE,
    SENSOR_PRESSURE,
    SENSOR_HUMIDITY,
    ACTUATOR_LED,
    ACTUATOR_MOTOR,
    COMMUNICATION_WIFI,
    COMMUNICATION_BLUETOOTH,
    UNKNOWN
};

enum class DeviceStatus {
    OK,
    ERROR,
    WARNING,
    DISCONNECTED,
    INITIALIZING,
    STOPPED,
    UNKNOWN
};

Sensor Service API
SensorManager Class
cpp

class SensorManager {
public:
    // Singleton access
    static SensorManager& getInstance();
    
    // Lifecycle
    bool initialize(const Json::Value& config = Json::nullValue);
    bool start();
    bool stop();
    bool isRunning() const;
    
    // Sensor management
    bool registerSensor(std::unique_ptr<SensorDriver> driver);
    bool unregisterSensor(const std::string& sensorId);
    SensorDriver* getSensor(const std::string& sensorId);
    std::vector<SensorDriver*> getSensorsByType(SensorType type);
    
    // Data operations
    bool readSensor(const std::string& sensorId, SensorReading& reading, int timeoutMs = 5000);
    std::vector<SensorReading> readAllSensors();
    SensorReading getLastReading(const std::string& sensorId) const;
    
    // Calibration
    bool calibrateSensor(const std::string& sensorId, const Json::Value& params);
    bool selfTestSensor(const std::string& sensorId, Json::Value& result);
    bool configureSensor(const std::string& sensorId, const Json::Value& config);
    
    // Discovery
    int discoverSensors(int timeoutMs = 10000);
    
    // Statistics
    Json::Value getStats() const;
    void resetStats();
    
    // Configuration
    bool saveConfig(const std::string& path) const;
    bool loadConfig(const std::string& path);
};

SensorDriver Class
cpp

class SensorDriver {
public:
    // Lifecycle
    virtual bool initialize(const Json::Value& config) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    
    // Data operations
    virtual bool readSample(SensorReading& reading) = 0;
    virtual bool calibrate(const Json::Value& params) = 0;
    virtual bool selfTest(Json::Value& result) = 0;
    virtual bool configure(const Json::Value& config) = 0;
    
    // Information
    virtual std::string getSensorId() const;
    virtual SensorType getType() const;
    virtual bool isConnected() const;
    virtual bool isRunning() const;
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    
    // Callbacks
    virtual void setDataCallback(DataCallback callback);
    virtual void setErrorCallback(ErrorCallback callback);
    virtual void setStatusCallback(StatusCallback callback);
};

Sensor Data Structures
cpp

struct SensorReading {
    SensorType type;
    std::string sensorId;
    std::map<std::string, double> values;
    std::chrono::system_clock::time_point timestamp;
    uint64_t sequence;
    bool isValid;
    std::map<std::string, std::string> metadata;
};

struct IMUData {
    struct Accelerometer {
        double x, y, z;
        double magnitude;
    };
    struct Gyroscope {
        double x, y, z;
        double magnitude;
    };
    struct Magnetometer {
        double x, y, z;
        double magnitude;
    };
    Accelerometer accel;
    Gyroscope gyro;
    Magnetometer mag;
    double temperature;
};

struct GPSData {
    double latitude;
    double longitude;
    double altitude;
    double speed;
    double heading;
    int satellites;
    bool hasFix;
};

struct TemperatureData {
    double temperature;
    double humidity;
    double dewPoint;
    double heatIndex;
};

struct PressureData {
    double pressure;
    double altitude;
    double seaLevelPressure;
    double temperature;
};

struct HumidityData {
    double humidity;
    double temperature;
    double dewPoint;
    double absoluteHumidity;
};

IPC Library API
IpcManager Class
cpp

class IpcManager {
public:
    // Singleton access
    static IpcManager& getInstance();
    
    // Initialization
    bool initialize(TransportType type, const std::string& address, 
                    const Json::Value& options = Json::nullValue);
    bool isInitialized() const;
    bool start();
    bool stop();
    bool isRunning() const;
    
    // Message handling
    void registerHandler(const std::string& topic, MessageHandler handler, int priority = 0);
    void unregisterHandler(const std::string& topic, MessageHandler handler);
    bool sendMessage(const IpcMessage& message, int timeoutMs = 0);
    IpcMessage sendRequest(const IpcMessage& message, int timeoutMs = 5000);
    void broadcast(const IpcMessage& message);
    
    // Transport types
    enum class TransportType {
        DBUS,
        UNIX_SOCKET,
        SHARED_MEMORY,
        MQTT,
        ZMQ,
        TCP,
        UDP
    };
    
    // Callbacks
    using MessageHandler = std::function<void(const IpcMessage&)>;
    using ErrorHandler = std::function<void(const std::string&)>;
    using ConnectionHandler = std::function<void(bool)>;
};

IpcMessage Class
cpp

class IpcMessage {
public:
    // Constructors
    IpcMessage();
    IpcMessage(const std::string& topic, const Json::Value& body);
    
    // Properties
    void setTopic(const std::string& topic);
    std::string getTopic() const;
    void setBody(const Json::Value& body);
    Json::Value getBody() const;
    void setType(int type);
    int getType() const;
    void setSender(const std::string& sender);
    std::string getSender() const;
    void setRecipient(const std::string& recipient);
    std::string getRecipient() const;
    void setCorrelationId(const std::string& correlationId);
    std::string getCorrelationId() const;
    void setTimestamp(std::chrono::system_clock::time_point timestamp);
    std::chrono::system_clock::time_point getTimestamp() const;
    
    // Serialization
    std::string serialize() const;
    bool deserialize(const std::string& data);
    
    // Types
    static const int TYPE_REQUEST = 1;
    static const int TYPE_RESPONSE = 2;
    static const int TYPE_NOTIFICATION = 3;
};

Logger Library API
Logger Class
cpp

class Logger {
public:
    // Singleton access
    static Logger& getInstance();
    
    // Initialization
    bool initialize(const std::string& configFile = "");
    bool initialize(const Config& config);
    
    // Logging
    void log(LogLevel level, const std::string& message, const Json::Value& context = Json::nullValue);
    void logEvent(const LogEvent& event);
    void logWithLocation(LogLevel level, const std::string& message,
                         const char* file, int line, const char* function,
                         const Json::Value& context = Json::nullValue);
    
    // Convenience methods
    void trace(const std::string& message, const Json::Value& context = Json::nullValue);
    void debug(const std::string& message, const Json::Value& context = Json::nullValue);
    void info(const std::string& message, const Json::Value& context = Json::nullValue);
    void warn(const std::string& message, const Json::Value& context = Json::nullValue);
    void error(const std::string& message, const Json::Value& context = Json::nullValue);
    void fatal(const std::string& message, const Json::Value& context = Json::nullValue);
    
    // Configuration
    void setLevel(LogLevel level);
    LogLevel getLevel() const;
    void setFormat(const std::string& format);
    void setColored(bool colored);
    void flush();
    
    // Sinks
    void addSink(std::unique_ptr<LogSink> sink);
    void removeSinks(const std::string& type);
    void clearSinks();
    
    // Statistics
    Json::Value getStats() const;
    void resetStats();
};

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    NOTICE = 3,
    WARN = 4,
    ERROR = 5,
    CRITICAL = 6,
    FATAL = 7,
    OFF = 8
};

Utils Library API
StringUtils Class
cpp

class StringUtils {
public:
    static std::string trim(const std::string& str);
    static std::string trimLeft(const std::string& str);
    static std::string trimRight(const std::string& str);
    static std::vector<std::string> split(const std::string& str, 
                                         const std::string& delimiter,
                                         size_t maxSplit = 0);
    static std::string join(const std::vector<std::string>& strings,
                           const std::string& delimiter);
    static std::string replace(const std::string& str,
                              const std::string& from,
                              const std::string& to);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static std::string toLower(const std::string& str);
    static std::string toUpper(const std::string& str);
    static bool contains(const std::string& str, const std::string& substr);
    static bool isValidEmail(const std::string& email);
    static bool isValidUrl(const std::string& url);
    static bool isValidIp(const std::string& ip);
    static std::string random(size_t length = 16, bool includeSpecial = false);
    static std::string hashSHA256(const std::string& str);
    static std::string toBase64(const std::string& str);
    static std::string fromBase64(const std::string& str);
    static std::string urlEncode(const std::string& str);
    static std::string urlDecode(const std::string& str);
};

FileUtils Class
cpp

class FileUtils {
public:
    static bool exists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isFile(const std::string& path);
    static bool isSymlink(const std::string& path);
    static size_t getSize(const std::string& path);
    static std::chrono::system_clock::time_point getModificationTime(const std::string& path);
    static std::string readToString(const std::string& path);
    static std::vector<uint8_t> readToBinary(const std::string& path);
    static bool writeString(const std::string& path, const std::string& content);
    static bool writeBinary(const std::string& path, const std::vector<uint8_t>& data);
    static bool copy(const std::string& source, const std::string& destination);
    static bool move(const std::string& source, const std::string& destination);
    static bool remove(const std::string& path);
    static bool createDirectory(const std::string& path, uint32_t permissions = 0755);
    static bool removeDirectory(const std::string& path);
    static std::vector<std::string> listFiles(const std::string& path,
                                             const std::string& pattern = "*",
                                             bool recursive = false);
    static std::string getBaseName(const std::string& path);
    static std::string getFileName(const std::string& path);
    static std::string getExtension(const std::string& path);
    static std::string getDirectory(const std::string& path);
    static std::string getAbsolutePath(const std::string& path);
    static std::string getCurrentDirectory();
    static std::string getTempDirectory();
    static std::string createTempFile(const std::string& prefix = "tmp");
    static std::string createTempDirectory(const std::string& prefix = "tmp");
};

TimeUtils Class
cpp

class TimeUtils {
public:
    static std::string currentTime(const std::string& format = "%Y-%m-%d %H:%M:%S");
    static int64_t currentTimestampMs();
    static int64_t currentTimestampUs();
    static int64_t currentTimestampNs();
    static std::string timestampToString(int64_t timestamp, 
                                        const std::string& format = "%Y-%m-%d %H:%M:%S");
    static int64_t stringToTimestamp(const std::string& timeStr,
                                    const std::string& format = "%Y-%m-%d %H:%M:%S");
    static int64_t parseDuration(const std::string& duration);
    static std::string formatDuration(int64_t milliseconds);
    static void sleep(int64_t milliseconds);
    
    class Stopwatch {
    public:
        Stopwatch();
        void start();
        void stop();
        void reset();
        int64_t elapsedMs() const;
        int64_t elapsedUs() const;
        int64_t elapsedNs() const;
        std::string toString() const;
    };
    
    class Timer {
    public:
        Timer();
        ~Timer();
        void start(int64_t intervalMs, std::function<void()> callback);
        void stop();
        bool isRunning() const;
    };
};

ThreadPool Class
cpp

class ThreadPool {
public:
    ThreadPool(size_t threadCount = 0);
    ~ThreadPool();
    
    template<typename F, typename... Args>
    auto submit(F&& task, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    
    template<typename F, typename... Args>
    auto submitPriority(int priority, F&& task, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
    
    void waitAll();
    void stop();
    void stopNow();
    bool isStopped() const;
    size_t getThreadCount() const;
    size_t getPendingCount() const;
    size_t getActiveCount() const;
    void setMaxQueueSize(size_t size);
};

Service Interfaces
D-Bus Interfaces
Sensor Service Interface
xml

<interface name="org.bbbai64.SensorService">
    <method name="ReadSensor">
        <arg type="s" name="sensorId" direction="in"/>
        <arg type="a{sv}" name="reading" direction="out"/>
    </method>
    <method name="ReadAllSensors">
        <arg type="aa{sv}" name="readings" direction="out"/>
    </method>
    <method name="CalibrateSensor">
        <arg type="s" name="sensorId" direction="in"/>
        <arg type="a{sv}" name="params" direction="in"/>
        <arg type="b" name="result" direction="out"/>
    </method>
    <signal name="DataAvailable">
        <arg type="s" name="sensorId"/>
        <arg type="a{sv}" name="data"/>
    </signal>
    <signal name="StatusChanged">
        <arg type="s" name="sensorId"/>
        <arg type="i" name="status"/>
    </signal>
</interface>

Device Manager Interface
xml

<interface name="org.bbbai64.DeviceManager">
    <method name="GetDevices">
        <arg type="aa{sv}" name="devices" direction="out"/>
    </method>
    <method name="EnableDevice">
        <arg type="s" name="deviceId" direction="in"/>
        <arg type="b" name="result" direction="out"/>
    </method>
    <method name="DisableDevice">
        <arg type="s" name="deviceId" direction="in"/>
        <arg type="b" name="result" direction="out"/>
    </method>
    <signal name="DeviceAdded">
        <arg type="s" name="deviceId"/>
        <arg type="a{sv}" name="info"/>
    </signal>
    <signal name="DeviceRemoved">
        <arg type="s" name="deviceId"/>
    </signal>
</interface>

Error Codes
Common Error Codes
cpp

enum class ErrorCode {
    SUCCESS = 0,
    GENERAL_ERROR = -1,
    NOT_FOUND = -2,
    NOT_INITIALIZED = -3,
    ALREADY_INITIALIZED = -4,
    INVALID_PARAMETER = -5,
    TIMEOUT = -6,
    PERMISSION_DENIED = -7,
    NOT_CONNECTED = -8,
    ALREADY_CONNECTED = -9,
    NOT_SUPPORTED = -10,
    BUSY = -11,
    OUT_OF_MEMORY = -12,
    IO_ERROR = -13,
    NETWORK_ERROR = -14,
    PROTOCOL_ERROR = -15,
    CONFIG_ERROR = -16,
    DRIVER_ERROR = -17,
    SENSOR_ERROR = -18,
    DEVICE_ERROR = -19,
    CALIBRATION_ERROR = -20,
    SELF_TEST_ERROR = -21
};

Version Information
API Version
Component	Version
Device Manager	1.0.0
Sensor Service	1.0.0
IPC Library	1.0.0
Logger Library	1.0.0
Utils Library	1.0.0
