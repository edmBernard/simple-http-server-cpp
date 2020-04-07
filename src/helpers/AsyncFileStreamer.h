#include <filesystem>
#include <string_view>

struct AsyncFileStreamer {

  std::map<std::string_view, AsyncFileReader *> asyncFileReaders;
  std::string root;

  AsyncFileStreamer(std::string root)
      : root(root) {
    // for all files in this path, init the map of AsyncFileReaders
    std::cout << "root : " << root << std::endl;
    updateRootCache();
  }

  void updateRootCache() {
    for (auto &p : std::filesystem::recursive_directory_iterator(root)) {
      std::string url = p.path().string().substr(root.length());
      if (url == "/index.html") {
        url = "/";
      }

      std::cout << "url available in root : " << url << std::endl;
      char *key = new char[url.length()];
      memcpy(key, url.data(), url.length());
      asyncFileReaders[std::string_view(key, url.length())] = new AsyncFileReader(p.path().string());
    }
  }

  template <bool SSL>
  void streamFile(uWS::HttpResponse<SSL> *res, std::string_view url) {
    auto it = asyncFileReaders.find(url);
    if (it == asyncFileReaders.end()) {
      std::cout << "Did not find file: " << url << std::endl;
    } else {
      streamFile(res, it->second);
    }
  }

  template <bool SSL>
  static void streamFile(uWS::HttpResponse<SSL> *res, AsyncFileReader *asyncFileReader) {
    /* Peek from cache */
    std::string_view chunk = asyncFileReader->peek(res->getWriteOffset());
    if (!chunk.length() || res->tryEnd(chunk, asyncFileReader->getFileSize()).first) {

      if (chunk.length() < asyncFileReader->getFileSize()) {
        asyncFileReader->request(res->getWriteOffset(), [res, asyncFileReader](std::string_view chunk) {
          /* We were aborted for some reason */
          if (!chunk.length()) {
            res->close();
          } else {
            AsyncFileStreamer::streamFile(res, asyncFileReader);
          }
        });
      }
    } else {
      /* We failed writing everything, so let's continue when we can */
      res->onWritable([res, asyncFileReader](int offset) {
           // här kan skiten avbrytas!

           AsyncFileStreamer::streamFile(res, asyncFileReader);
           // todo: I don't really know what this is supposed to mean?
           return false;
         })
          ->onAborted([]() {
            std::cout << "ABORTED!" << std::endl;
          });
    }
  }
};
