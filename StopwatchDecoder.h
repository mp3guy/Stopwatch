/*
 * StopwatchDecoder.h
 *
 *  Created on: 23 Nov 2011
 *      Author: thomas
 */

#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

class StopwatchDecoder {
 public:
  static std::pair<uint64_t, std::vector<std::pair<std::string, float>>> decodePacket(
      const unsigned char* data,
      int size) {
    const char* stringData = (const char*)&data[sizeof(int) + sizeof(uint64_t)];

    int totalLength = sizeof(int) + sizeof(uint64_t);

    std::pair<uint64_t, std::vector<std::pair<std::string, float>>> values;

    while (totalLength < size) {
      std::pair<std::string, float> nextTiming;

      nextTiming.first = std::string(stringData);
      stringData += nextTiming.first.length() + 1;

      nextTiming.second = *((float*)stringData);
      stringData += sizeof(float);

      totalLength += nextTiming.first.length() + 1 + sizeof(float);

      values.second.push_back(nextTiming);
    }

    values.first = *((uint64_t*)&data[sizeof(int)]);

    return values;
  }
};
