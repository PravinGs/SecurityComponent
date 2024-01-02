#ifndef MQTT_clientENT
#define MQTT_clientENT

#include "model/mqtt_model.hpp"

class Imqtt_client
{
public:
	virtual void start(const mqtt_entity &entity) = 0;
	virtual ~Imqtt_client() {}
};

class mqtt_client: public Imqtt_client
{
private:
	mqtt::async_client *client = nullptr;

private:
	void publish_response(const std::string &response, const std::string &original_topic);

	void handle_message(const mqtt::const_message_ptr &msg);

public:

	void start(const mqtt_entity &entity);
};

#endif