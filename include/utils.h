#pragma once

#include <core_init.h>


void printTime(const char* label, f64 CPUTime, u64 _CPUTimerFreq);
void printTime(const char* label, u64 CPUTime, u64 _CPUTimerFreq);
void printTimeWithBandwith(const char* label, f64 CPUTime, u64 _CPUTimerFreq, u64 byteCount);
void printTimeWithBandwith(const char* label, u64 CPUTime, u64 _CPUTimerFreq, u64 byteCount);
