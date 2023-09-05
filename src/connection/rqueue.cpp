#include "connection/rqueue.hpp"

Queue::Queue(string caKey, string clientCert, string clientKey)
{
    this->_handler = new Handler(this->_loop, caKey, clientCert, clientKey);
    this->_connection = new AMQP::TcpConnection(this->_handler, AMQP::Address("amqps://guest:guest@localhost/"));
}

int Queue::_parseJSON(string jsonFile, string &json)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string parseErrors;
    Json::StreamWriterBuilder writerBuilder;

    std::ifstream file(jsonFile);
    if (!file.is_open())
    {
        AgentUtils::writeLog(FILE_ERROR + jsonFile, FAILED);
        return FAILED;
    }

    string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::istringstream jsonStream(jsonString);
    if (!Json::parseFromStream(builder, jsonStream, &root, &parseErrors))
    {
        AgentUtils::writeLog("Failed to parse the JSON file: " + jsonFile, FAILED);
        return FAILED;
    }

    json = Json::writeString(writerBuilder, root);
    return SUCCESS;
}

int Queue::send(string jsonFile, string queue)
{
    string jsonString;
    if (_parseJSON(jsonFile, jsonString) == FAILED || jsonString.length() == 0)
        return FAILED;
    if (_checkConnection() == FAILED)
    {
        AgentUtils::writeLog("RabbitMQ connecetion not alive", FAILED);
        return FAILED;
    }
    return _publish(jsonString, queue);
}

int Queue::_checkConnection()
{
    if (this->_connection->closed())
    {
        this->_connection = new AMQP::TcpConnection(this->_handler, AMQP::Address("amqps://guest:guest@localhost/"));
    }
    return FAILED ? this->_connection->closed() : SUCCESS;
}

int Queue::_publish(const string jsonString, string queue)
{
    AMQP::TcpChannel channel(this->_connection);
    int result = FAILED;
    try
    {
        channel.onError([](const char *error)
                        { throw std::invalid_argument(error); });
        channel.declareQueue(queue, AMQP::durable).onSuccess([&, jsonString]()
                                                             {
            channel.publish("", queue, jsonString);
            channel.close();
            this->_connection->close(); });
        ev_run(this->_loop);

        result = SUCCESS;
    }
    catch (exception &e)
    {
        string error = e.what();
        AgentUtils::writeLog(error, FAILED);
    }
    return result;
}

Queue::~Queue()
{
    if (!this->_connection->closed())
    {
        this->_connection->close();
    }
    delete this->_connection;
    delete this->_handler;
}