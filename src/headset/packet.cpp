#include "headset/packet.hpp"
#include <numeric>
#include <stdlib.h>

OutPacket::OutPacket(std::string const &data) : data(data) {}

char *OutPacket::encode(int *len) {
  *len = sizeof(sOutPacket) + this->data.size() + 1;
  sOutPacket *packet = (sOutPacket *)malloc(*len);
  packet->flag[0] = 0x56;
  packet->flag[1] = 0x5A;
  packet->length = this->data.size();
  packet->type = PNNL;
  std::copy(this->data.begin(), this->data.end(), packet->data);
  int const checksum_idx = this->data.size();
  uint8_t checksum = std::reduce<uint8_t *>(
      reinterpret_cast<uint8_t *>(&(*this->data.begin())),
      reinterpret_cast<uint8_t *>(&(*this->data.end())), 0);
  packet->data[checksum_idx] = static_cast<char>(checksum);
  return reinterpret_cast<char *>(packet);
}

InPacket::InPacket(char *in_data)
    : counter(get_counter_(in_data)), data(get_data_(in_data)),
      timestamp(get_timestamp_(in_data)){};

int const InPacket::get_counter_(char *data) {
  sInPacket *packet = reinterpret_cast<sInPacket *>(data);
  return static_cast<int>(packet->counter);
}

std::string const &InPacket::get_data_(char *data) {
  sInPacket *packet = reinterpret_cast<sInPacket *>(data);
  return std::move(std::string(packet->data, packet->length));
}

std::chrono::milliseconds const &InPacket::get_timestamp_(char *data) {
  sInPacket *packet = reinterpret_cast<sInPacket *>(data);
  uint32_t bet = packet->timestamp;
  uint32_t milliseconds = BYTESWAP(bet);
  return std::move(std::chrono::milliseconds(milliseconds));
}
