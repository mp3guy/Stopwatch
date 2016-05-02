/*
 * StopwatchDecoder.h
 *
 *  Created on: 23 Nov 2011
 *      Author: thomas
 */

#ifndef STOPWATCHDECODER_H_
#define STOPWATCHDECODER_H_

#include <utility>
#include <vector>
#include <string>
#include <iostream>

class StopwatchDecoder
{
    public:
        static std::pair<unsigned long long int, std::vector<std::pair<std::string, float> > > decodePacket(const unsigned char * data, int size)
        {
            const char * stringData = (const char *)&data[sizeof(int) + sizeof(unsigned long long int)];

            int totalLength = sizeof(int) + sizeof(unsigned long long int);

            std::pair<unsigned long long int, std::vector<std::pair<std::string, float> > > values;

            while(totalLength < size)
            {
                std::pair<std::string, float> nextTiming;

                nextTiming.first = std::string(stringData);
                stringData += nextTiming.first.length() + 1;

                nextTiming.second = *((float *)stringData);
                stringData += sizeof(float);

                totalLength += nextTiming.first.length() + 1 + sizeof(float);

                values.second.push_back(nextTiming);
            }

            values.first = *((unsigned long long int *)&data[sizeof(int)]);

            return values;
        }
};

#endif /* STOPWATCHDECODER_H_ */
