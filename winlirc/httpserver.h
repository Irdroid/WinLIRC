#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <string>

class HttpServer
{
public:
    HttpServer();
    HttpServer(HttpServer const&) = delete;
    ~HttpServer();

    HttpServer& operator=(HttpServer const&) = delete;

    using Handler = std::function<int(struct mg_connection* conn)>;

    HttpServer& registerHandler(
        std::string const& method,
        std::string const& pathPrefix,
        Handler const& handler)
    {
        if (method == "GET")
            getRoutes_.insert(std::make_pair(pathPrefix, handler));
        return *this;
    }

    void run();
    void stop();

private:
    static int ev_handler(struct mg_connection* conn, enum mg_event ev);

    int get(struct mg_connection* conn);
    int put(struct mg_connection* conn);
    int post(struct mg_connection* conn);
    int del(struct mg_connection* conn);

    struct mg_server* server_;
    std::atomic<bool> stopped_;
    std::atomic<bool> running_;
    std::map<std::string, Handler> getRoutes_;
    // TODO: other methods
};
