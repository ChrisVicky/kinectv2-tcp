#include <iostream>

#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/logger.h>
#include <libfreenect2/packet_pipeline.h>

#include <chrono>

#include "tcp/tcp.hpp"

using namespace std;

const std::string SERVER_ADDRESS = "tcp://192.168.0.190:1883";
const std::string ID = "TxCamera";

const std::string address = "192.168.0.190";
const int port = 12345;

bool protonect_shutdown = false;

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

  if (!tcp.Connect(address, port)) {
    std::cerr << "Unable to connect to server" << std::endl;
    exit(-1);
  }
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

  dev->stop();
  dev->close();
  std::cout << "Goodbye World!" << std::endl;
  return 0;
}
