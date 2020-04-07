#include <string>
#include <string_view>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <sstream>

/* This is just a very simple and inefficient demo of async responses,
 * please do roll your own variant or use a database or Node.js's async
 * features instead of this really bad demo */
struct AsyncFileReader {
private:
  /* The cache we have in memory for this file */
  std::string cache;
  std::size_t cacheOffset;
  bool hasCache;

  /* The pending async file read (yes we only support one pending read) */
  std::function<void(std::string_view)> pendingReadCb;

  std::size_t fileSize;
  std::string fileName;
  std::ifstream fin;
  uWS::Loop *loop;

public:
  /* Construct a demo async. file reader for fileName */
  AsyncFileReader(std::string fileName)
      : fileName(fileName) {
    fin.open(fileName, std::ios::binary);

    // get fileSize
    fin.seekg(0, fin.end);
    fileSize = fin.tellg();

    // cache up 1 mb!
    cache.resize(1024 * 1024);

    //std::cout << "Caching 1 MB at offset = " << 0 << std::endl;
    fin.seekg(0, fin.beg);
    fin.read(cache.data(), cache.length());
    cacheOffset = 0;
    hasCache = true;

    // get loop for thread

    loop = uWS::Loop::get();
  }

  /* Returns any data already cached for this offset */
  std::string_view peek(std::size_t offset) {
    /* Did we hit the cache? */
    if (hasCache && offset >= cacheOffset && ((offset - cacheOffset) < cache.length())) {
      /* Cache hit */
      std::size_t chunkSize = std::min<std::size_t>(fileSize - offset, cache.length() - offset + cacheOffset);

      return std::string_view(cache.data() + offset - cacheOffset, chunkSize);
    } else {
      /* Cache miss */
      return std::string_view(nullptr, 0);
    }
  }

  /* Asynchronously request more data at offset */
  void request(std::size_t offset, std::function<void(std::string_view)> cb) {

    // in this case, what do we do?
    // we need to queue up this chunk request and callback!
    // if queue is full, either block or close the connection via abort!
    if (!hasCache) {
      // already requesting a chunk!
      std::cout << "ERROR: already requesting a chunk!" << std::endl;
      return;
    }

    // disable cache
    hasCache = false;

    std::async(std::launch::async, [this, cb, offset]() {

      if (!fin.good()) {
        fin.close();
        fin.open(fileName, std::ios::binary);
      }
      fin.seekg(offset, fin.beg);
      fin.read(cache.data(), cache.length());

      cacheOffset = offset;
      hasCache = true;

      std::size_t chunkSize = std::min<std::size_t>(cache.length(), fileSize - offset);
      cb(std::string_view(cache.data(), chunkSize));

    });
  }

  /* Abort any pending async. request */
  void abort() {
  }

  std::size_t getFileSize() {
    return fileSize;
  }
};
