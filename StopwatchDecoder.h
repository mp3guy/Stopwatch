/*
 * StopwatchDecoder.h
 *
 *  Created on: 23 Nov 2011
 *      Author: thomas
 */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

class StopwatchDecoder {
 public:
  static std::pair<uint64_t, std::vector<std::pair<std::string, float>>> decodePacket(
      const uint8_t* data,
      int totalLength,
      std::map<uint64_t, std::map<std::string, uint64_t>>& signatureToNameToTicksUs) {
    // Skip the first 4 bytes (int size of total packet) and next 8 bytes (signature for packet)
    const char* stringData = (const char*)(data + sizeof(int) + sizeof(uint64_t));

    uint64_t signature = 0;
    memcpy(&signature, data + sizeof(int), sizeof(uint64_t));

    // Count what we've processed so far
    int processedLength = sizeof(int) + sizeof(uint64_t);

    std::pair<uint64_t, std::vector<std::pair<std::string, float>>> signatureNameDurationsMs = {
        signature, {}};
    std::vector<std::pair<std::string, uint64_t>> nameTicksUs;
    std::vector<std::pair<std::string, uint64_t>> nameTocksUs;

    while (processedLength < totalLength) {
      // Read the measurement type
      const uint8_t type = *stringData++;

      // Exploit the fact std::string will stop at the next null terminator
      const std::string name(stringData);
      stringData += name.length() + 1;

      auto routeDatum = [&](auto& collection) {
        using ValueType =
            typename std::remove_reference_t<decltype(collection)>::value_type::second_type;
        ValueType value = {};
        memcpy(&value, stringData, sizeof(ValueType));
        stringData += sizeof(ValueType);
        processedLength += sizeof(uint8_t) + (name.length() + 1) + sizeof(ValueType);
        collection.emplace_back(name, value);
      };

      if (type == 0) {
        routeDatum(signatureNameDurationsMs.second);
      } else if (type == 1) {
        routeDatum(nameTicksUs);
      } else if (type == 2) {
        routeDatum(nameTocksUs);
      }
    }

    // Store any ticks we received that we need to match against future tocks
    for (const auto& [name, tickUs] : nameTicksUs) {
      signatureToNameToTicksUs[signature][name] = tickUs;
    }

    // Try to match tocks from other process's ticks
    for (const auto& [tockName, tockUs] : nameTocksUs) {
      for (auto& [sig, nameToTicksUs] : signatureToNameToTicksUs) {
        const auto matchingNameAndTickUs = nameToTicksUs.find(tockName);

        if (matchingNameAndTickUs != nameToTicksUs.end()) {
          const uint64_t tickUs = matchingNameAndTickUs->second;
          nameToTicksUs.erase(matchingNameAndTickUs);

          float durationMs = (float)(tockUs - tickUs) / 1000.0f;

          if (durationMs > 0) {
            signatureNameDurationsMs.second.emplace_back(tockName, durationMs);
          }
        }
      }
    }

    return signatureNameDurationsMs;
  }
};
