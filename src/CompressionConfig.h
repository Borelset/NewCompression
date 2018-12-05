//
// Created by borelset on 11/28/18.
//

#ifndef NC_COMPRESSIONCONFIG_H
#define NC_COMPRESSIONCONFIG_H

#define FIT_METHOD_COUNT 8

enum class PrecisionType{
    Absolution,
    Relative,
};

struct CompressionConfig{
    PrecisionType precisionType = PrecisionType::Absolution;
    double range = 1.0;
    unsigned fitMethodCount = FIT_METHOD_COUNT;
    int diffRange = 0xffff / 2;
};

#endif //NC_COMPRESSIONCONFIG_H
