// Copyright 2020 Burylov Denis <burylov01@mail.ru>

#include <boost/log/keywords/file_name.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/log/keywords/rotation_size.hpp>
#include <boost/log/keywords/time_based_rotation.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <csignal>
#include <hash.hpp>
namespace logging = boost::log;
namespace sinks = logging::sinks;
namespace keywords = logging::keywords;
namespace expressions = logging::expressions;

void init() {
  boost::shared_ptr<logging::core> core = logging::core::get();
  // logs to file for info
  boost::shared_ptr<sinks::text_file_backend> backend1 =
      boost::make_shared<sinks::text_file_backend>(
          keywords::file_name = "logs/file_info_%5N.log",
          keywords::rotation_size = 5 * 1024 * 1024,
          keywords::format = "[%TimeStamp%]: %Message%",
          keywords::time_based_rotation =
              sinks::file::rotation_at_time_point(12, 0, 0));

  typedef sinks::synchronous_sink<sinks::text_file_backend> sink_file;
  boost::shared_ptr<sink_file> sink1(new sink_file(backend1));
  sink1->set_filter(logging::trivial::severity >= logging::trivial::info);
  core->add_sink(sink1);

  // logs to file for trace
  boost::shared_ptr<sinks::text_file_backend> backend2 =
      boost::make_shared<sinks::text_file_backend>(
          keywords::file_name = "logs/file_trace_%5N.log",
          keywords::rotation_size = 5 * 1024 * 1024,
          keywords::format = "[%TimeStamp%]: %Message%",
          keywords::time_based_rotation =
              sinks::file::rotation_at_time_point(12, 0, 0));

  boost::shared_ptr<sink_file> sink2(new sink_file(backend2));
  sink2->set_filter(logging::trivial::severity <= logging::trivial::trace);
  core->add_sink(sink2);

  // logs to console
  boost::shared_ptr<sinks::text_ostream_backend> backend3 =
      boost::make_shared<sinks::text_ostream_backend>();
  backend3->add_stream(
      boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

  typedef sinks::synchronous_sink<sinks::text_ostream_backend> sink_console;
  boost::shared_ptr<sink_console> sink3(new sink_console(backend3));
  sink3->set_filter(logging::trivial::severity >= logging::trivial::info);
  core->add_sink(sink3);
}

int main(int argc, char* argv[]) {
  init();
  auto threads_count = boost::thread::hardware_concurrency();
  std::string file;
  if (argc >= 2) {
    if (std::stoul(argv[1]) > 0) threads_count = std::stoul(argv[1]);
    if (argc == 3) file = argv[2];
  }
  BOOST_LOG_TRIVIAL(info) << threads_count << " threads";
  BOOST_LOG_TRIVIAL(info) << "File: " << file;
  static ::std::atomic_bool shutdown = false;
#ifdef __APPLE__
  {
    sig_t handler = [](int const signal) {
      BOOST_LOG_TRIVIAL(info)
          << "Shutting down due to signal " << signal << std::endl;
      shutdown = true;
    };
    bsd_signal(SIGINT, handler);
    bsd_signal(SIGSTOP, handler);
    bsd_signal(SIGTERM, handler);
  }
#else
  {
    __sighandler_t handler = [](int const signal) {
      BOOST_LOG_TRIVIAL(info)
          << "Shutting down due to signal " << signal << std::endl;
      shutdown = true;
    };
    sysv_signal(SIGINT, handler);
    sysv_signal(SIGSTOP, handler);
    sysv_signal(SIGTERM, handler);
  }
#endif
  auto* p_shutdown = &shutdown;
  std::vector<boost::thread> threads;
  for (size_t i = 0; i < threads_count; i++) {
    threads.emplace_back(boost::thread{find_hash, p_shutdown});
  }
  for (size_t i = 0; i < threads_count; i++) {
    threads[i].join();
  }
  if (!file.empty()) {
    std::ofstream out(file);
    write_to_file(out, hash_array);
    out.close();
  }
  return 0;
}
