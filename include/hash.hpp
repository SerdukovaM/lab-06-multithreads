// Copyright 2020 Burylov Denis <burylov01@mail.ru>

#ifndef INCLUDE_HASH_HPP_
#define INCLUDE_HASH_HPP_

#include <picosha2.h>

#include <boost/log/trivial.hpp>
#include <boost/thread/mutex.hpp>
#include <ctime>
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <iomanip>

std::mutex mut;

class Hash {
  std::string data_, hash_;
  int64_t time_;

 public:
  explicit Hash(const std::string& data)
      : data_(data), hash_(picosha2::hash256_hex_string(data)), time_(0) {}

  bool check() { return hash_.substr(hash_.size() - 4, 4) == "0000"; }

  void set_time(const int64_t& time) { time_ = time; }

  const std::string& data() { return data_; }

  const std::string& hash() { return hash_; }

  const int64_t& time() { return time_; }
};
std::vector<Hash> hash_array;
void find_hash(std::atomic_bool* shutdown) {
  srand(time(NULL));

  auto begin = std::chrono::steady_clock::now();
  for (;;) {
    Hash hash{std::to_string(rand())};
    if (hash.check()) {
      BOOST_LOG_TRIVIAL(info)
          << "Thread id: " << std::this_thread::get_id()
          << " Data: " << hash.data() << " Hash: " << hash.hash();
      auto end = std::chrono::steady_clock::now();
      mut.lock();
      hash.set_time(
          std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
              .count());
      hash_array.push_back(hash);
      mut.unlock();
      begin = std::chrono::steady_clock::now();
      // break;
    } else {
      BOOST_LOG_TRIVIAL(trace)
          << "Thread id: " << std::this_thread::get_id()
          << " Data: " << hash.data() << " Hash: " << hash.hash();
    }
    if (*shutdown) {
      break;
    }
  }
}

nlohmann::json to_json_hash(const std::vector<Hash>& vec) {
  nlohmann::json vector = nlohmann::json::array();
  for (auto hash : vec) {
    nlohmann::json j;
    j["timestamp"] = hash.time();
    j["hash"] = hash.hash();
    j["data"] = hash.data();
    vector.push_back(j);
  }
  return vector;
}

void write_to_file(std::ofstream& out, const std::vector<Hash>& vec) {
  nlohmann::json j = to_json_hash(vec);
  out << std::setw(4) << j;
}

#endif  // INCLUDE_HASH_HPP_
