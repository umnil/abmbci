#ifndef INCLUDE_HEADSET_PACKET_HPP_
#define INCLUDE_HEADSET_PACKET_HPP_
#include <chrono>
#include <stdint.h>
#include <string>
#define HEADER_SIZE 9
#define PNNL 1
#define BYTE1(x) (x & 0xFF)
#define BYTE2(x) (BYTE1(x >> 8))
#define BYTE3(x) (BYTE2(x >> 8))
#define BYTE4(x) (BYTE3(x >> 8))
#define BYTESWAP(x) ((BYTE1(x) << 24) | (BYTE2(x) << 16) | (BYTE3(x) << 8) | BYTE4(x))

#pragma pack(push, 1)
typedef struct OUTPACKET {
  uint8_t flag[2];
  uint16_t length;
  uint8_t type;
  char data[];
} sOutPacket;

typedef struct INPACKET {
  uint8_t flag[2];
  uint8_t counter;
  uint32_t timestamp;
  uint16_t length;
  uint8_t type;
  char data[];
} sInPacket;
#pragma pack(pop)

class OutPacket {
public:
  OutPacket(std::string const &data);
  char *encode(int *);
  std::string data;
};

class InPacket {
public:
  InPacket(char *in_data);
  int const counter;
  std::string const data;
  std::chrono::milliseconds const timestamp;

private:
  int const get_counter_(char *data);
  std::string const &get_data_(char *data);
  std::chrono::milliseconds const get_timestamp_(char *data);
};

#endif /* INCLUDE_HEADSET_PACKET_HPP_ */
