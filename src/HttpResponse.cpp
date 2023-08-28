#include <iostream>
#include <sstream>
#include <cstring>

#include "soundbag/HttpResponse.hpp"

using namespace soundbag;

HttpResponse::HttpResponse(int status) {
  this->status = status;
  this->headers["Connection"] = "Close";
}

HttpResponse::HttpResponse(const std::string& str) {
  this->status = 200;
  this->body = str;
  this->headers["Content-Type"] = "text/plain";
  this->headers["Cache-Control"] = "max-age=600";
  this->headers["Connection"] = "Close";
}

HttpResponse::HttpResponse(const nlohmann::json& json) {
  this->status = 200;
  this->body = json.dump();
  this->headers["Content-Type"] = "application/json";
  this->headers["Cache-Control"] = "max-age=600";
  this->headers["Connection"] = "Close";
}

static constexpr unsigned char MAGIC_JPG[] = {0xFF, 0xD8};
static constexpr unsigned char MAGIC_PNG[] = {0x89, 'P', 'N', 'G'};
static constexpr unsigned char MAGIC_MP4[] = {0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D, 0x00, 0x00, 0x02, 0x00};

HttpResponse::HttpResponse(std::ifstream& file, const std::string& ext) {
  this->status = 200;

  std::istreambuf_iterator<char> begin(file);
  std::istreambuf_iterator<char> end;
  this->body = std::string(begin, end);

  if (ext == "html") {
    this->headers["Content-Type"] = "text/html";
  }
  else if (ext == "js") {
    this->headers["Content-Type"] = "text/javascript";
  }
  else if (ext == "css") {
    this->headers["Content-Type"] = "text/css";
  }
  else if (ext == "jpg" || ext == "jpeg" || (body.size() >= sizeof(MAGIC_JPG) && memcmp(body.c_str(), MAGIC_JPG, sizeof(MAGIC_JPG)) == 0)) {
    this->headers["Content-Type"] = "image/jpeg";
  }
  else if (ext == "png" || (body.size() >= sizeof(MAGIC_PNG) && memcmp(body.c_str(), MAGIC_PNG, sizeof(MAGIC_PNG)) == 0)) {
    this->headers["Content-Type"] = "image/png";
  }
  else if (ext == "gif" || (body.size() >= 3 && memcmp(body.c_str(), "GIF", 3) == 0)) {
    this->headers["Content-Type"] = "image/png";
  }
  else if (ext == "bmp" || (body.size() >= 2 && memcmp(body.c_str(), "BM", 2) == 0)) {
    this->headers["Content-Type"] = "image/bmp";
  }
  else if (ext == "mp3" || (body.size() >= 3 && memcmp(body.c_str(), "ID3", 3) == 0)) {
    this->headers["Content-Type"] = "audio/mpeg";
  }
  else if (ext == "m4a") {
    this->headers["Content-Type"] = "audio/aac";
  }
  else if (ext == "mp4" || (body.size() >= sizeof(MAGIC_MP4) && memcmp(body.c_str(), MAGIC_MP4, sizeof(MAGIC_MP4)) == 0)) {
    this->headers["Content-Type"] = "video/mp4";
  }
  else if (ext == "pdf" || memcmp(body.c_str(), "%PDF", 4) == 0) {
    this->headers["Content-Type"] = "application/pdf";
  }
  else if (file.binary) {
    this->headers["Content-Type"] = "application/octet-stream";
  }
  else {
    this->headers["Content-Type"] = "text/plain";
  }

  this->headers["Cache-Control"] = "max-age=604800";
  this->headers["Connection"] = "Close";
}

std::string HttpResponse::serialize() {
  std::ostringstream oss;
  oss << "HTTP/1.1 " << status << " " << statusMessage() << "\r\n";
  for (auto e : headers) {
    oss << e.first << ": " << e.second << "\r\n";
  }
  oss << "\r\n" << body;

  return oss.str();
}

std::string HttpResponse::statusMessage() {
  switch (status) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 102: return "Processing";
    case 103: return "Early Hints";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 207: return "Multi-Status";
    case 208: return "Already Reported";
    case 226: return "IM Used";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 306: return "";
    case 307: return "Temporary Redirect";
    case 308: return "Permanent Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Payload Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 418: return "I'm a teapot";
    case 419: return "Invalid CSRF tokens";
    case 421: return "Misdirected Request";
    case 422: return "Unprocessable Content";
    case 423: return "Locked";
    case 424: return "Failed Dependency";
    case 425: return "Too Early";
    case 426: return "Upgrade Required";
    case 428: return "Precondition Required";
    case 429: return "Too Many Requests";
    case 431: return "Request Header Fields Too Large";
    case 451: return "Unavailable For Legal Reasons";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    case 506: return "Variant Also Negotiates";
    case 507: return "Insufficient Storage";
    case 508: return "Loop Detected";
    case 510: return "Not Extended";
    case 511: return "Network Authentication Required";
    default: return "OK";
  }
}
