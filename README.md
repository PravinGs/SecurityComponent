# **IOT Agent Software**

## **Description**
* Agent, as an application, will run on a variety of system devices (computers and IOT devices). It is capable of collecting information about processes running on the device, including log files, scheduling services, backup and restore of configuration files, cloud connectivity, and TLS communication to the gateway for any remote update to the device.

## **Modules**
* Created the Logger module for sending critical log information to the cloud server for auditing.

* Created a queueing module for sending log and process information to the gateway in the absence of the cloud server.

* Created the Monitor module to collect information about processes running on the device.

* Created a Scheduler module to periodically start the services.

* Created a Watcher module for file monitoring to backup the updated files.

* Admins can change agent-software configuration files from the remote machine using TLS1.3.

* Created LogAnalyzer module to analyze logs and its severity level based on the configured rules.

* Created RootCheck Module to check untrusted file locations based on the pre identified malware paths.

## **Technologies used**
* c++17
* AMQP-CPP 
* Openssl 
* jsoncpp
* eventloop
* pcre2


