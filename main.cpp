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

/* #define STB_IMAGE_WRITE_IMPLEMENTATION */
/* #include "stb_image_write.h" */

#include <chrono>
#include <zlib.h>

#include "parser/parser.hpp"
#include "tcp/tcp.hpp"

#define TCP_IMAGE
/* #define MQTT_IMAGE */

using namespace std;

const std::string SERVER_ADDRESS = "tcp://localhost:1883";
const std::string ID = ">>Tester<<";

bool protonect_shutdown = false;

mqtt::async_client *client;

void inform(std::string topic);
void sigint_handler(int s) { protonect_shutdown = true; }

/* void stbiWriteToVector(void *context, void *data, int size); */
/* std::vector<unsigned char> frameToPNGData(libfreenect2::Frame *frame); */
/* std::string compressData(const std::vector<unsigned char> &png_data); */

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
    std::cout << "Message arrived" << std::endl;
    std::cout << "Topic: " << msg->get_topic() << std::endl;
    std::cout << "Payload: " << msg->to_string() << std::endl;

    // WARN: No checking on the `/topic`
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
  });

  auto lwt =
      mqtt::message("/exit", "<<<" + ID + " was disconnected>>>", 1, false);

  auto connOpts =
      mqtt::connect_options_builder().will(std::move(lwt)).finalize();

  try {
    std::cout << "Connecting" << std::endl;
    client->connect(connOpts)->wait();
    std::cout << "Connected" << std::endl;

    client->subscribe("/ipport", 0);
  } catch (const mqtt::exception &exc) {
    std::cout << "ERROR: Unable to connect to MQTT server: " << SERVER_ADDRESS
              << " " << exc << std::endl;
    exit(1);
  }
  // ==================== MQTT INIT ====================

  // ==================== libfreenect2 ====================
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

  std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
  std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;

  // ==================== libfreenect2 ====================

  inform("/online");

  // ==================== Main Loop ====================
  protonect_shutdown = false;
  double fps = 0;
  auto last = std::chrono::high_resolution_clock::now();
  while (!protonect_shutdown) {
    listener.waitForNewFrame(frames);
    libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];

    if (rgb != nullptr) {
#ifdef IMAGE_MQTT
      try {
        mqtt::message_ptr pubmsg = mqtt::make_message(
            "/image", rgb->data, rgb->width * rgb->height * 4);
        pubmsg->set_qos(0);
        client->publish(pubmsg);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      } catch (const mqtt::exception &exc) {
        cerr << exc.what() << endl;
        break;
      }
#elif defined(TCP_IMAGE)
      try {
        if (!tcp.Put(rgb->data, rgb->width * rgb->height * 4)) {
          std::cerr << "Failed to send image data" << std::endl;
        }
      } catch (const std::exception &exc) {
        std::cerr << exc.what() << std::endl;
        break;
      }
#endif
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
  std::cout << "Publishing Online message to " << topic << std::endl;
  mqtt::message_ptr msg = mqtt::make_message(
      topic, "{ \"id\": \"camera\", \"type\": \"tx\", \"status\": \"online\"}");
  msg->set_qos(1);
  client->publish(msg);
}

/* std::vector<unsigned char> frameToPNGData(libfreenect2::Frame *frame) { */
/*   std::vector<unsigned char> pngData; */
/**/
/*   if (frame == nullptr || frame->data == nullptr) { */
/*     std::cerr << "Frame is null or contains no data." << std::endl; */
/*     return pngData; // 返回空vector */
/*   } */
/**/
/*   // 假设frame->data是RGB格式的数据，每个像素3个字节 */
/*   int width = frame->width; */
/*   int height = frame->height; */
/**/
/*   // 使用stb库函数，将数据写入vector */
/*   if (!stbi_write_png_to_func(stbiWriteToVector, &pngData, width, height, 3,
 */
/*                               frame->data, 0)) { */
/*     std::cerr << "Failed to convert frame to PNG data." << std::endl; */
/*   } */
/**/
/*   return pngData; */
/* } */
/**/
/* void stbiWriteToVector(void *context, void *data, int size) { */
/*   // 将数据追加到传入的vector中 */
/*   std::vector<unsigned char> *buffer = */
/*       reinterpret_cast<std::vector<unsigned char> *>(context); */
/*   unsigned char *pData = reinterpret_cast<unsigned char *>(data); */
/*   buffer->insert(buffer->end(), pData, pData + size); */
/* } */
/**/
/* std::string compressData(const std::vector<unsigned char> &png_data) { */
/*   uLongf compressedSize = compressBound(png_data.size()); */
/*   std::vector<unsigned char> compressedData(compressedSize); */
/**/
/*   int result = compress2(compressedData.data(), &compressedSize, */
/*                          png_data.data(), png_data.size(),
 * Z_BEST_COMPRESSION); */
/*   if (result != Z_OK) { */
/*     std::cerr << "Compression failed" << std::endl; */
/*     return ""; */
/*   } */
/**/
/*   compressedData.resize(compressedSize); */
/*   return std::string(compressedData.begin(), compressedData.end()); */
/* } */
