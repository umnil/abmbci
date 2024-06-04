#include "headset/packet.hpp"
#include <iostream>
#include <numeric>
#include <stdlib.h>

OutPacket::OutPacket(std::string const &data) : data(data) {}

char *OutPacket::encode(int *len) {
  *len = sizeof(sOutPacket) + this->data.size() + 1;
  sOutPacket *packet = (sOutPacket *)malloc(*len);
  packet->flag[0] = 0x56;
  packet->flag[1] = 0x5A;
  packet->length = BYTESWAP16(this->data.size());
  packet->type = PNNL;
  std::copy(this->data.begin(), this->data.end(), packet->data);
  int const checksum_idx = this->data.size();
  packet->data[checksum_idx] = 0;
  uint8_t checksum =
      0xff - std::reduce<uint8_t *>(
                 reinterpret_cast<uint8_t *>(&packet->length),
                 reinterpret_cast<uint8_t *>(&packet->data[checksum_idx]), 0);
  packet->data[checksum_idx] = static_cast<char>(checksum);
  return reinterpret_cast<char *>(packet);
}

InPacket::InPacket(char *in_data, int len)
    : counter(get_counter_(in_data)),
      data(reinterpret_cast<uint8_t *>(in_data),
           reinterpret_cast<uint8_t *>(in_data) + len),
      timestamp(get_timestamp_(in_data)), userdata(get_data_(in_data)){};

int const InPacket::get_counter_(char *data) {
  if (data == nullptr)
    return 0;
  sInPacket *packet = reinterpret_cast<sInPacket *>(data);
  return static_cast<int>(packet->counter);
}

std::string const InPacket::get_data_(char *data) {
  if (data == nullptr)
    return std::string();
  sInPacket *packet = reinterpret_cast<sInPacket *>(data);
  return std::string(packet->data, BYTESWAP16(packet->length) - 2);
}

std::chrono::milliseconds const InPacket::get_timestamp_(char *data) {
  if (data == nullptr)
    return std::chrono::milliseconds(0);
  sInPacket *packet = reinterpret_cast<sInPacket *>(data);
  uint32_t bet = packet->timestamp;
  uint32_t milliseconds = BYTESWAP32(bet);
  return std::chrono::milliseconds(milliseconds);
}
