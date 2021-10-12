// This is a simple HTTP(S) web server much like Python's SimpleHTTPServer

#include <string>
#include <iostream>

#include <cxxopts.hpp>
#include <uwebsockets/App.h>

/* Helpers for this example */
#include "helpers/AsyncFileReader.h"
#include "helpers/AsyncFileStreamer.h"

/* optparse */
#define OPTPARSE_IMPLEMENTATION

int main(int argc, char **argv) {
  try {
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
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
    }

    if (!result.count("directory")) {
      std::cout << "Error: directory argument required" << std::endl;
      return 1;
    }

    std::string root = result["directory"].as<std::string>();
    int port = result["port"].as<int>();

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

  } catch (const cxxopts::OptionException &e) {
    std::cout << "Error: parsing options: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
