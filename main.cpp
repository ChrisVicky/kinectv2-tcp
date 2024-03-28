#include <iostream>
#include <mqtt/async_client.h>
#include <mqtt/connect_options.h>
#include <mqtt/create_options.h>
#include <mqtt/server_response.h>

#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/logger.h>
#include <libfreenect2/packet_pipeline.h>

#include <mqtt/client.h>

#include <chrono>

#include "parser/parser.hpp"
#include "tcp/tcp.hpp"

using namespace std;

const std::string SERVER_ADDRESS = "tcp://localhost:1883";
const std::string ID = "TxCamera";

bool protonect_shutdown = false;

mqtt::async_client *client;

void inform(std::string topic);
void sigint_handler(int s) { protonect_shutdown = true; }

int main() {
  std::cout << "Hello World!" << std::endl;

  // ==================== TCP INIT ====================
  TCPClient tcp;
  if (!tcp.Init()) {
    std::cerr << "Unable to initialize TCP Client" << std::endl;
    return -1;
  }
  // ==================== TCP INIT ====================

  // ==================== MQTT INIT ====================
  client = new mqtt::async_client(SERVER_ADDRESS, ID);

  client->set_connection_lost_handler([](const std::string &cause) {
    std::cout << "Connection Lost" << std::endl;
    if (!cause.empty())
      std::cout << "Cause: " << cause << std::endl;
  });

  client->set_connected_handler([](const std::string &cause) {
    std::cout << "Connected to " << SERVER_ADDRESS << std::endl;
  });

  client->set_message_callback([&tcp](mqtt::const_message_ptr msg) {
    std::cout << "[MQTT] Message arrived" << std::endl;
    std::cout << "[MQTT] Topic: " << msg->get_topic() << std::endl;
    std::cout << "[MQTT] Payload: " << msg->to_string() << std::endl;

    std::string topic = msg->get_topic();
    if (topic == "/ipport") {
      std::string address;
      int port;
      std::string pld = msg->to_string();
      if (!parseAddressIp(pld, address, port)) {
        std::cerr << "Error parsing address" << std::endl;
        exit(-1);
      }
      std::cout << "Address: " << address << std::endl;
      std::cout << "Port: " << port << std::endl;

      if (!tcp.Connect(address, port)) {
        std::cerr << "Unable to connect to server" << std::endl;
        exit(-1);
      }
    } else if (topic == "/exit") {
      protonect_shutdown = true;
    } else if (topic == "/client/WiFi-Tx/" + ID + "/start") {
      // TODO: Add Wi-Fi Tx Control
      // Start Tx Transmissions (Using Bash) --> See Wi-Fi Tx
    } else if (topic == "/client/WiFi-Tx/" + ID + "/stop") {
      // Stop Tx Transmission Threads
    }
  });

  auto lwt =
      mqtt::message("/exit", "<<<" + ID + " was disconnected>>>", 1, false);

  auto connOpts =
      mqtt::connect_options_builder().will(std::move(lwt)).finalize();

  // Connecting MQTT
  try {
    std::cout << "[MQTT] Connecting (" << SERVER_ADDRESS << ")" << std::endl;
    client->connect(connOpts)->wait();
    std::cout << "[MQTT] Connected" << std::endl;

    client->subscribe("/ipport", 0);
  } catch (const mqtt::exception &exc) {
    std::cout << "ERROR: Unable to connect to MQTT server: " << SERVER_ADDRESS
              << " " << exc << std::endl;
    exit(1);
  }
  // ==================== MQTT INIT ====================

  // ==================== libfreenect2 INIT ====================
  libfreenect2::Freenect2 freenect2;
  libfreenect2::Freenect2Device *dev = freenect2.openDefaultDevice();

  if (dev == nullptr) {
    std::cout << "failure opening device!" << std::endl;
    return -1;
  }

  signal(SIGINT, sigint_handler);

  //! [listeners]
  libfreenect2::SyncMultiFrameListener listener(libfreenect2::Frame::Color);
  libfreenect2::FrameMap frames;

  dev->setColorFrameListener(&listener);
  dev->start();

  std::cout << "[libfreenect2] device serial: " << dev->getSerialNumber()
            << std::endl;
  std::cout << "[libfreenect2] device firmware: " << dev->getFirmwareVersion()
            << std::endl;

  // ==================== libfreenect2 ====================

  inform("/online");

  // ==================== Main Loop ====================
  protonect_shutdown = false;
  double fps = 0;
  auto last = std::chrono::high_resolution_clock::now();
  while (!protonect_shutdown) {
    listener.waitForNewFrame(frames);
    libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
    // NOTE: The Format is BGRX
    // Further modifications shall be done before being displayed by
    // common RGB displayers

    if (rgb != nullptr) {
      try {
        if (!tcp.Put(rgb->data, rgb->width * rgb->height * 4)) {
          std::cerr << "Failed to send image data" << std::endl;
        }
      } catch (const std::exception &exc) {
        std::cerr << exc.what() << std::endl;
        break;
      }
      fps += 1;
    }

    if (fps > 100) {
      auto now = std::chrono::high_resolution_clock::now();
      double elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(now - last).count();
      std::cout << "FPS: " << fps / elapsed << std::endl;
      last = now;
      fps = 0;
    }
    listener.release(frames);
  }

  inform("/offline");

  dev->stop();
  dev->close();
  std::cout << "Goodbye World!" << std::endl;
  return 0;
}

void inform(std::string topic) {
  if (client == nullptr) {
    std::cerr << "MQTT Client is not initialized" << std::endl;
    return;
  }
  std::cout << "[MQTT] Publishing message to " << topic << std::endl;
  mqtt::message_ptr msg = mqtt::make_message(
      topic,
      "{ \"id\": \"" + ID + "\", \"type\": \"tx\", \"status\": \"online\"}");
  msg->set_qos(1);
  client->publish(msg);
}
