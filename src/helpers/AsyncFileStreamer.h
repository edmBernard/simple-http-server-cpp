#include <algorithm>
#include <filesystem>
#include <string_view>

#include <spdlog/spdlog.h>

#include <helpers/AsyncFileReader.h>

struct AsyncFileStreamer {

  std::map<std::string_view, AsyncFileReader *> asyncFileReaders;
  std::string root;

  AsyncFileStreamer(std::string root)
      : root(root) {
    // for all files in this path, init the map of AsyncFileReaders
    spdlog::info("root : {}", root);
    updateRootCache();
  }

  void updateRootCache() {
    for (auto &p : std::filesystem::recursive_directory_iterator(root)) {
      if (std::filesystem::is_directory(p.path())) {
        continue;
      }
      std::string url = "/" + std::filesystem::relative(p.path(), std::filesystem::path(root)).generic_string();

      spdlog::info("url available in root : {}", url);
      char *key = new char[url.length()];
      memcpy(key, url.data(), url.length());
      asyncFileReaders[std::string_view(key, url.length())] = new AsyncFileReader(p.path().string());
    }
  }

  template <bool SSL>
  void streamFile(uWS::HttpResponse<SSL> *res, std::string_view url) {
    auto it = url == "/" ? asyncFileReaders.find("/index.html") : asyncFileReaders.find(url);
    if (it == asyncFileReaders.end()) {
      spdlog::info("Did not find url: {}", url);
    } else {
      streamFile(res, it->second);
    }
  }

  template <bool SSL>
  static void streamFile(uWS::HttpResponse<SSL> *res, AsyncFileReader *asyncFileReader) {
    // Peek from cache
    std::string_view chunk = asyncFileReader->peek(res->getWriteOffset());

    if (!chunk.length() || res->tryEnd(chunk, asyncFileReader->getFileSize()).first) {
      if (chunk.length() < asyncFileReader->getFileSize()) {

        asyncFileReader->request(res->getWriteOffset(), [res, asyncFileReader](std::string_view chunk) {
          // We were aborted for some reason
          if (!chunk.length()) {
            res->close();
          } else {
            AsyncFileStreamer::streamFile(res, asyncFileReader);
          }
        });
      }
    } else {

      // We failed writing everything, so let's continue when we can
      res->onWritable([res, asyncFileReader](int offset) {
           AsyncFileStreamer::streamFile(res, asyncFileReader);
           return false;
         })
          ->onAborted([]() {
            spdlog::info("Aborted request");
          });
    }
  }
};
