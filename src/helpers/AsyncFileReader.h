#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>

struct AsyncFileReader {
private:
  // The cache we have in memory for this file
  std::string cache;
  std::size_t cacheOffset = 0;
  bool hasCache = false;
  bool isValid = false;

  std::future<void> pendingReadCallback;

  std::size_t fileSize;
  std::string filename;
  std::ifstream fin;
  uWS::Loop *loop;

public:
  AsyncFileReader(std::string filename)
      : filename(filename) {

    if (!std::filesystem::exists(filename)) {
      spdlog::error("File don't exist: {}", filename);
      return;
    }

    if (std::filesystem::is_directory(filename)) {
      spdlog::error("File is a directory: {}", filename);
      return;
    }

    fileSize = std::filesystem::file_size(filename);
    // cache up 1 mb!
    cache.resize(1024 * 1024);

    // fill cache with beginning of the file
    fin.open(filename, std::ios::binary);
    fin.read(cache.data(), cache.length());

    hasCache = true;
    isValid = true;

    // get loop for thread
    loop = uWS::Loop::get();
  }

  // Returns any data already cached for this offset
  std::string_view peek(std::size_t offset) {
    if (!isValid) {
      spdlog::error("Trying to access invalid file: {}", filename);
      return std::string_view(nullptr, 0);
    }

    // Did we hit in the cache
    if (hasCache && offset >= cacheOffset && ((offset - cacheOffset) < cache.length())) {
      std::size_t cacheSupLimit = std::min<std::size_t>(fileSize, cacheOffset + cache.length());
      return std::string_view(cache.data() + offset - cacheOffset, cacheSupLimit - offset);

    } else {
      return std::string_view(nullptr, 0);
    }
  }

  // Asynchronously request more data at offset
  void request(std::size_t offset, std::function<void(std::string_view)> callback) {

    // in this case, what do we do?
    // we need to queue up this chunk request and callback!
    // if queue is full, either block or close the connection via abort!
    if (!hasCache) {
      spdlog::error("Already requesting a chunk!");
      return;
    }

    // disable cache
    hasCache = false;

    pendingReadCallback = std::async(std::launch::async, [this, callback, offset]() {
      if (!fin.good()) {
        // if something wrong we reopen it
        fin.close();
        fin.open(filename, std::ios::binary);
      }
      fin.seekg(offset, fin.beg);
      fin.read(cache.data(), cache.length());

      cacheOffset = offset;

      // pass the callback execution in uWS loop
      loop->defer([this, callback, offset]() {
        int chunkSize = std::min<int>(cache.length(), fileSize - offset);

        if (chunkSize == 0) {
          spdlog::warn("Zero size chunk");
        }

        if (chunkSize != cache.length()) {
          spdlog::warn("Less than 1 MB cache");
        }

        hasCache = true;
        callback(std::string_view(cache.data(), chunkSize));
      });
    });
  }

  // Abort any pending async. request
  void abort() {
  }

  std::size_t getFileSize() {
    return fileSize;
  }
};