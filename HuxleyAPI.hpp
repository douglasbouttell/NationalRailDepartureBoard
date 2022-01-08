#ifndef __HUXLEY_API_HPP__
#define __HUXLEY_API_HPP__

#include <Logger.h>
#include <Print.h>
#include <HTTPClient.h>

const String EMPTY_STRING = String("");
const String DEFAULT_BASE_URL = String("huxley.apphb.com");
const String DEFAULT_ACCESS_TOKEN = String("DA1C7740-9DA0-11E4-80E6-A920340000B1");

class HuxleyAPI {
  private:
    String baseURL;
    String accessToken;
    HTTPClient http;
    String body;
    int code;

  public:
    HuxleyAPI() 
      : baseURL(DEFAULT_BASE_URL)
      , accessToken(DEFAULT_ACCESS_TOKEN) 
      , code(-1)
    {}

    HuxleyAPI(const char* baseURL)
      : baseURL(String(baseURL))
      , accessToken(DEFAULT_ACCESS_TOKEN)
      , code(-1)
    {}

    HuxleyAPI(const char* baseURL, const char* accessToken)
      : baseURL(String(baseURL))
      , accessToken(String(accessToken))
      , code(-1)
    {}

    String getBody() {
      return this->body;
    }

    String getErrorString() {
      return this->http.errorToString(code);
    }

    boolean get(const char* stationCrs) {
      String url = "http://" 
        + this->baseURL
        + "/departures/" 
        + String(stationCrs) 
        + "/20"
        + "?accessToken="
        + this->accessToken;
      
      this->http.begin(url);
      this->code = http.GET();
      
      if(this->code == HTTP_CODE_OK) {
        this->body = this->http.getString();
        this->http.end();
        return true;
      } else {
        this->http.end();
        this->body = String("");
        return false;
      }
    }  
};

#endif // __HUXLEY_API_HPP__
