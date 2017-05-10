#include "scanner.h"
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

const float32 PI = 3.14159265;

RosLaserScanData::RosLaserScanData() {
  angle_min = 0.0;
  angle_max = 2 * PI;
  angle_increment = 0.001;
  time_increment = 0.0;
  scan_time = 0.0;
  range_min = 0.0;
  range_max = -1.0;
}
  
RosMapMetaData::RosMapMetaData() {
  map_load_time_sec = time(0);
  map_load_time_nsec = 0;
  resolution = 1.0;
  width = 0;
  height = 0;
  position = {0.0, 0.0, 0.0};
  orient_x = orient_y = orient_z = orient_w = 0.0;
}

bool Scanner::is_correct() const {
  return correct_;
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
  correct_ = false;
  if (parameters_.angle_min > parameters_.angle_max ||
      parameters_.angle_increment <= 0) {
    std::cerr << "incorrect angle paramemers" << std::endl;
  }
  else if (parameters_.range_min < 0.0 ||
           (parameters_.range_max < parameters_.range_min &&
            parameters_.range_max != -1.0)) {
    std::cerr << "incorrect range paramemers" << std::endl;  
  }
  else if (!grid_in) {
    std::cerr << "unable to open grid file" << std::endl;
  }
  else {
    grid_in.ignore(sizeof(RosHeader));
    grid_in.read((char *) &info_, sizeof(info_));
    data_.resize(info_.width * info_.height);
    grid_in.read((char *) &data_[0], data_.size() * sizeof(data_[0]));
    position_.x -= info_.position.x;
    position_.y -= info_.position.y;
    if (position_.x < 0.0 || position_.x > info_.width  * info_.resolution ||
        position_.y < 0.0 || position_.y > info_.height * info_.resolution) {
      std::cerr << "incorrect scanner position" << std::endl;
    }
    else
      correct_ = true;
  }
}

float32 Scanner::dist_to_cell(float32 angle, ScannerDirection direction,
                              int32 row, int32 col) const {
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

float32 Scanner::get_range(float32 angle) {
  int32 col = position_.x / info_.resolution;
  int32 row = position_.y / info_.resolution;
  ScannerDirection direction = NONE;
  while (0 <= col && col < (int32) info_.width &&
         0 <= row && row < (int32) info_.height) {
    float32 dist = dist_to_cell(angle, direction, row, col);
    if (data_[row * info_.width + col] > 0)
      return dist;
    else if (data_[row * info_.width + col] == -1)
      return -1.0;
    else if (parameters_.range_min <= dist &&
             (dist <= parameters_.range_max || parameters_.range_max < 0.0))
      data_[row * info_.width + col] = -2;
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
      tmp = angle_normalize(tmp);
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
      float32 tmp = atan2(row * info_.resolution - position_.y,
                          col * info_.resolution - position_.x);
      tmp = angle_normalize(tmp);
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
      float32 tmp = atan2(row      * info_.resolution - position_.y, 
                         (col + 1) * info_.resolution - position_.x);
      tmp = angle_normalize(tmp);
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

float32 Scanner::angle_normalize(float32 angle) const {
  if (angle > 2 * PI)
    angle -= 2 * PI;
  if (angle <= 0.0)
    angle += 2 * PI;
  return angle;
}

void Scanner::scan() {
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
  for (float32 angle = angle_min; angle <= angle_max;
                                  angle += parameters_.angle_increment) {
    float32 scan_angle = angle;
    scan_angle = angle_normalize(scan_angle);
    ranges_.push_back(get_range(scan_angle));
    if ((ranges_.back() > parameters_.range_max &&
         parameters_.range_max > 0.0) ||
        ranges_.back() < parameters_.range_min)
      ranges_.back() = -1.0;
  }
  if (parameters_.range_max < 0.0)
    parameters_.range_max = *std::max_element(ranges_.begin(), ranges_.end());
}

void Scanner::write(std::ofstream &msg_out) const {
  RosHeader header = {0, (uint32) time(0), 0, "1"};
  msg_out.write((char *) &header, sizeof(header));
  msg_out.write((char *) &parameters_, sizeof(parameters_));
  msg_out.write((char *) &ranges_[0], ranges_.size() * sizeof(ranges_[0]));
  for (uint32 i = 0; i < ranges_.size() * sizeof(ranges_[0]); i++)
    msg_out.put(0);
}

void Scanner::visualize(std::ofstream &pgm_out) {
  pgm_out << "P5" << std::endl;
  pgm_out << info_.width << " " << info_.height << std::endl;
  pgm_out << 255 << std::endl;
  uint32 pos_row = position_.y / info_.resolution;
  uint32 pos_col = position_.x / info_.resolution;
  uint32 pos_cell = pos_row * info_.width + pos_col;
  int8 tmp = 100;
  std::swap(data_[pos_cell], tmp);
  for (int8 cell : data_) {
    if (cell > 0)
      pgm_out.put(0);
    else if (cell == 0)
      pgm_out.put(255);
    else if (cell == -1)
      pgm_out.put(204);
    else
      pgm_out.put(100);
  }
  std::swap(data_[pos_cell], tmp);
}
