#include "service/mqtt_service.hpp"

void mqtt_client::publish_response(const std::string &response, const std::string &original_topic)
{
    try
    {
        auto pubmsg = mqtt::make_message(original_topic, response);
        pubmsg->set_qos(1);
        client->publish(pubmsg)->wait();
        std::cout << "Response published: " << response << std::endl;
    }
    catch (const mqtt::exception &e)
    {
        std::cerr << "Error publishing response: " << e.what() << std::endl;
    }
}

void mqtt_client::handle_message(const mqtt::const_message_ptr &msg)
{
    std::cout << "Received message on topic: " << msg->get_topic() << " - Payload: " << msg->get_payload_str() << std::endl;
    // Parse and assign tasks to the received input command.
    // publish_response("Response: Message received", msg->get_topic());
}

void mqtt_client::start(const mqtt_entity &entity)
{
    client = new mqtt::async_client(entity.conn_string, entity.client_id);

    auto conn_opts = mqtt::connect_options_builder().clean_session(true).finalize();

    try
    {
        client->start_consuming();

        agent_utils::write_log("mqtt_client: start: " + entity.client_id + " : connecting to the mqtt server", DEBUG);
        auto tok = client->connect(conn_opts);

        // Getting the connect response will block waiting for the
        // connection to complete.
        auto rsp = tok->get_connect_response();

        // If there is no session present, then we need to subscribe, but if
        // there is a session, then the server remembers us and our
        // subscriptions.
        if (!rsp.is_session_present())
        {
            for (const string &topic : entity.topics)
            {
                client->subscribe(topic, entity.qos)->wait();
                agent_utils::write_log("mqtt_client: start: " + entity.client_id + ": subscribe to " + topic, DEBUG);
            }
        }

        while (true)
        {
            auto msg = client->consume_message();
            if (!msg)
            {
                break;
            }
            handle_message(msg);
        }

        // If we're here, the client was almost certainly disconnected.
        // But we check, just to make sure.
        if (client->is_connected())
        {
            agent_utils::write_log("mqtt_client: start: " + entity.client_id + " : Shutting down and disconnecting from the MQTT server...", DEBUG);
            for (const auto &topic : entity.topics)
            {
                client->unsubscribe(topic)->wait();
            }
            client->stop_consuming();
            client->disconnect()->wait();
        }
        else
        {
            agent_utils::write_log("mqtt_client: start: " + entity.client_id + " disconnected", DEBUG);
        }
    }
    catch (const mqtt::exception &e)
    {
        string error = e.what();
        agent_utils::write_log("mqtt_client: start: mqtt_service excepiton: " + error, ERROR);
        std::cerr << e.what() << '\n';
    }
}