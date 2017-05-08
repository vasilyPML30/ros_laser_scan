#ifndef SCANNER_H_
#define SCANNER_H_

#include <fstream>
#include <cstdint>
#include <vector>

typedef double float64;
typedef float float32;
typedef uint32_t uint32;
typedef int32_t int32;
typedef char int8;
typedef unsigned char uint8;

enum ScannerDirection {NONE, RIGHT, UP, LEFT, DOWN};


#pragma pack(push, 1)

struct RosHeader {
  uint32 seq;
  uint32 time_sec;
  uint32 time_nsec;
  char frame_id[2];
};

struct RosPoint {
  float64 x;
  float64 y;
  float64 z;
};

struct RosMapMetaData {
  RosMapMetaData();
  uint32 map_load_time_sec;
  uint32 map_load_time_nsec;
  float32 resolution;
  uint32 width;
  uint32 height;
  RosPoint position;
  float64 orient_x, orient_y, orient_z, orient_w;
};

struct RosLaserScanData {
  RosLaserScanData();
  float32 angle_min;
  float32 angle_max;
  float32 angle_increment;
  float32 time_increment;
  float32 scan_time;
  float32 range_min;
  float32 range_max;
};

#pragma pack(pop)

class Scanner {
public:
  Scanner(std::ifstream &yaml_in);
  void scan();
  void write(std::ofstream &msg_out) const;

private:
  float32 get_range(float32 angle) const;
  float32 dist_to_cell(float32 angle, ScannerDirection direction,
                     int32 row, int32 col) const;


  RosLaserScanData parameters_;
  RosPoint position_;
  RosMapMetaData info_;
  std::vector<int8> data_;
  std::vector<float32> ranges_;
};

#endif