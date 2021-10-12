// This is a simple HTTP(S) web server much like Python's SimpleHTTPServer

#include <iostream>
#include <string>

#include <cxxopts.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

#include <uwebsockets/App.h>

#include "helpers/AsyncFileReader.h"
#include "helpers/AsyncFileStreamer.h"

/* optparse */
#define OPTPARSE_IMPLEMENTATION

int main(int argc, char **argv) try {

  spdlog::cfg::load_env_levels();

  // =================================================================================================
  // CLI
  cxxopts::Options options(argv[0], "A simple http server");
  options.positional_help("[optional args]").show_positional_help();

  // clang-format off
  options.add_options()("help", "Print help")
    ("p,port", "port", cxxopts::value<int>()->default_value("80"), "PORT")
    ("d,directory", "directory to serve", cxxopts::value<std::string>(), "DIRECTORY")
    ;
  // clang-format on
  options.parse_positional({"directory"});
  auto clo = options.parse(argc, argv);

  if (clo.count("help")) {
    fmt::print("{}", options.help());
    return EXIT_SUCCESS;
  }

  if (!clo.count("directory")) {
    spdlog::error("Directory argument required");
    fmt::print("{}", options.help());
    return EXIT_FAILURE;
  }

  std::string root = clo["directory"].as<std::string>();
  int port = clo["port"].as<int>();

  AsyncFileStreamer asyncFileStreamer(root);

  uWS::App().get("/*", [&asyncFileStreamer](auto *res, auto *req) {
              asyncFileStreamer.streamFile(res, req->getUrl());
              res->end();
            })
      .listen(port, [port, root](auto *token) {
        if (token) {
          std::cout << "Serving " << root << " over HTTP a " << port << std::endl;
        }
      })
      .run();

  return EXIT_SUCCESS;

} catch (const cxxopts::OptionException &e) {
  spdlog::error("Parsing options : {}", e.what());
  return EXIT_FAILURE;

} catch (const std::exception &e) {
  spdlog::error("{}", e.what());
  return EXIT_FAILURE;
}