#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t nat;

typedef uint8_t  nat8;
typedef uint16_t nat16;
typedef uint32_t nat32;
typedef uint64_t nat64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  byte;
typedef uint16_t doublebyte;
typedef uint32_t quadbyte;
typedef uint64_t octabyte;

#ifdef __linux__
  #define RED     "\x1B[31m"
  #define GREEN   "\x1B[32m"
  #define YELLOW  "\x1B[33m"
  #define BLUE    "\x1B[34m"
  #define MAGENTA "\x1B[35m"
  #define CYAN    "\x1B[36m"
  #define RESET   "\x1B[0m"
#else
  #define RED     ""
  #define GREEN   ""
  #define YELLOW  ""
  #define BLUE    ""
  #define MAGENTA ""
  #define CYAN    ""
  #define RESET   ""
#endif

#ifdef DEBUG
  #define DEBUG_INFO(Message, ...) fprintf(stderr, BLUE "[DEBUG INFO] %s:%d:%s(): " RESET Message ".\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
  #define DEBUG_ERROR(Message, ...) do { fprintf(stderr, RED "[DEBUG ERROR] %s:%d:%s(): " RESET Message ".\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); exit(1); } while(0)
#else
  #define DEBUG_INFO(Message, ...)
  #define DEBUG_ERROR(Message, ...)
#endif

#define INFO(Message, ...) do { fprintf(stderr, BLUE "[INFO] " RESET Message ".\n", ##__VA_ARGS__); } while(0)
#define WARN(Message, ...) do { fprintf(stderr, YELLOW "[WARNING] " RESET Message ".\n", ##__VA_ARGS__); } while(0)
#define ERROR(Message, ...) do { fprintf(stderr, RED "[ERROR] " RESET Message ".\n", ##__VA_ARGS__); exit(1); } while(0)