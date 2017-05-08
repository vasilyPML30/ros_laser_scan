#include "scanner.h"
#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <iostream>

const float32 PI = 3.14159265;
const float32 EPSILON = 1e-5;

RosLaserScanData::RosLaserScanData() {
  angle_min = 0.0;
  angle_max = 2 * PI;
  angle_increment = 0.01;
  time_increment = 0.0;
  scan_time = 0.0;
  range_min = 0.0;
  range_max = 1e9;
}
  
RosMapMetaData::RosMapMetaData() {
  map_load_time_sec = time(0);
  map_load_time_nsec = 0;
  resolution = 1.0;
  width = 0;
  height = 0;
  position = {0, 0, 0};
  orient_x = orient_y = orient_z = orient_w = 0.0;
}

float32 Scanner::dist_to_cell(float32 angle, ScannerDirection direction,
                              int32 row, int32 col) const {
  if (angle == 0)
    std::cout << col * info_.resolution << " " << position_.x << " " << position_.y << std::endl;
  switch (direction) {
    case RIGHT:
      return (col * info_.resolution - position_.x) / cos(angle);
    case UP:
      return (row * info_.resolution - position_.y) / sin(angle);
    case LEFT:
      return ((col + 1) * info_.resolution - position_.x) / cos(angle);
    case DOWN:
      return ((row + 1) * info_.resolution - position_.y) / sin(angle);
    default:
      return 0;
  }
}

float32 Scanner::get_range(float32 angle) const {
  int32 col = position_.x / info_.resolution;
  int32 row = position_.y / info_.resolution;
  ScannerDirection direction = NONE;
  while (0 <= col && col < (int32) info_.width &&
         0 <= row && row < (int32) info_.height) {
    if (data_[row * info_.width + col] > 0)
      return dist_to_cell(angle, direction, row, col);
    if (angle < PI / 2) {
      float32 tmp = atan2((row + 1) * info_.resolution - position_.y, 
                        (col + 1) * info_.resolution - position_.x);
      if (angle < tmp) {
        col++;
        direction = RIGHT;
      }
      else {
        row++;
        direction = UP;
      }
    }
    else if (angle < PI) {
      float32 tmp = atan2((row + 1) * info_.resolution - position_.y, 
                                col      * info_.resolution - position_.x);
      if (angle < tmp) {
        row++;
        direction = UP;
      }
      else {
        col--;
        direction = LEFT;
      }
    }
    else if (angle < 3 * PI / 2) {
      float32 tmp = 2 * PI + atan2(row * info_.resolution - position_.y,
                               col * info_.resolution - position_.x);
      if (angle < tmp) {
        col--;
        direction = LEFT;
      }
      else {
        row--;
        direction = DOWN;
      }
    }
    else {
      float32 tmp = 2 * PI + atan2(row      * info_.resolution - position_.y, 
                                  (col + 1) * info_.resolution - position_.x);
      if (angle < tmp) {
        row--;
        direction = DOWN;
      }
      else {
        col++;
        direction = RIGHT;
      }
    }
  }
  return -1.0;
}

Scanner::Scanner(std::ifstream &yaml_in) {
  std::string param, grid_file_name;
  while (yaml_in >> param) {
    if (param == "angle_min:")
      yaml_in >> parameters_.angle_min;
    else if (param == "angle_max:")
      yaml_in >> parameters_.angle_max;
    else if (param == "angle_increment:")
      yaml_in >> parameters_.angle_increment;
    else if (param == "range_min:")
      yaml_in >> parameters_.range_min;
    else if (param == "range_max:")
      yaml_in >> parameters_.range_max;
    else if (param == "grid:")
      yaml_in >> grid_file_name;
    else if (param == "position:") {
      char tmp;
      yaml_in >> tmp >>  position_.x >> tmp >> position_.y >> tmp;
      yaml_in.ignore(2);
    }
    else if (param[0] == '#')
      yaml_in.unget();
      getline(yaml_in, param);
  }
  std::ifstream grid_in(grid_file_name);
  grid_in.ignore(sizeof(RosHeader));
  grid_in.read((char *) &info_, sizeof(info_));
  data_.resize(info_.width * info_.height);
  grid_in.read((char *) &data_[0], data_.size() * sizeof(data_[0]));
  position_.x -= info_.position.x;
  position_.y -= info_.position.y;
}

void Scanner::scan() {
  /*
  for (uint32_t i = 0; i < info_.height; i++) {
    for (uint32_t j = 0; j < info_.width; j++)
      std::cout << (int)data_[i * info_.width + j] << " ";
    std::cout << std::endl;
  }
  */
  float32 angle_min = parameters_.angle_min;
  float32 angle_max = parameters_.angle_max;
  if (angle_min < 0) {
    int32 delta = (-angle_min / (2 * PI));
    angle_min += 2 * PI * delta;
    angle_max += 2 * PI * delta;
  }
  if (angle_min < 0) {
    angle_min += 2 * PI;
    angle_max += 2 * PI;
  }
  if (angle_min > 2 * PI) {
    int32 delta = (int32)(angle_min / (2 * PI)) - 1;
    angle_min -= 2 * PI * delta;
    angle_max -= 2 * PI * delta;
  }
  if (angle_min > 2 * PI) {
    angle_min -= 2 * PI;
    angle_max -= 2 * PI;
  }
  angle_max  = std::min(angle_max, angle_min + 2 * PI);
  for (float32 angle = parameters_.angle_min; angle <= parameters_.angle_max;
                                       angle += parameters_.angle_increment) {
    float32 scan_angle = angle;
    if (scan_angle > 2 * PI)
      scan_angle -= 2 * PI;
    std::cout << angle << ": ";
    ranges_.push_back(get_range(scan_angle));
    if (ranges_.back() > parameters_.range_max ||
        ranges_.back() < parameters_.range_min)
      ranges_.back() = -1.0;
    std::cout << ranges_.back() << std::endl;
  }
}

void Scanner::write(std::ofstream &msg_out) const {
  RosHeader header = {0, (uint32) time(0), 0, "1"};
  msg_out.write((char *) &header, sizeof(header));
  msg_out.write((char *) &parameters_, sizeof(parameters_));
  msg_out.write((char *) &ranges_[0], ranges_.size() * sizeof(ranges_[0]));
  for (uint32 i = 0; i < ranges_.size() * sizeof(ranges_[0]); i++)
    msg_out.put(0);
}
