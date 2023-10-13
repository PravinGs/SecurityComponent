

// void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
//     if (message->payloadlen) {
//         std::cout << "Received message on topic: " << message->topic << "\n";
//         std::cout << "Message: " << std::string((char *)message->payload, message->payloadlen) << "\n";
//     } else {
//         std::cout << "Empty message received on topic: " << message->topic << "\n";
//     }
// }

// int main() {
    
//     mosquitto_lib_init();

//     // Create a Mosquitto client instance
//     struct mosquitto *mosq = mosquitto_new("C++Client", true, nullptr);

//     if (!mosq) {
//         std::cerr << "Error: Unable to create Mosquitto client instance.\n";
//         return 1;
//     }

//     mosquitto_message_callback_set(mosq, message_callback);

//     // Connect to the MQTT broker
//     int rc = mosquitto_connect(mosq, "localhost", 1883, 60);

//     if (rc) {
//         std::cerr << "Error: Unable to connect to the MQTT broker. Return code: " << rc << "\n";
//         mosquitto_destroy(mosq);
//         mosquitto_lib_cleanup();
//         return 1;
//     }

//     // Subscribe to a topic
//     mosquitto_subscribe(mosq, nullptr, "example/topic", 0);

//     // Loop and handle messages
//     while (true) {
//         rc = mosquitto_loop(mosq, -1, 1);
//         if (rc) {
//             std::cerr << "Error: Unable to loop and handle messages. Return code: " << rc << "\n";
//             break;
//         }
//     }

//     // Disconnect and clean up
//     mosquitto_disconnect(mosq);
//     mosquitto_destroy(mosq);
//     mosquitto_lib_cleanup();

//     return 0;
// }
