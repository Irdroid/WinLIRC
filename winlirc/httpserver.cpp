#include "httpserver.h"

#include "mongoose/mongoose.h"
#include <Windows.h>

HttpServer::HttpServer()
    : stopped_(false)
    , running_(false)
    , server_(mg_create_server(this, ev_handler))
{
    mg_set_option(server_, "listening_port", "8080");
    mg_set_option(server_, "document_root", "C:\\Users\\lego\\projects\\winlirc\\winlirc\\ui");
    mg_set_option(server_, "index_files", "index.html");
}

HttpServer::~HttpServer()
{
    stop();
    while (running_)
    {
        ::Sleep(100);
    }
    mg_destroy_server(&server_);
}

int HttpServer::ev_handler(mg_connection* conn, mg_event ev)
{
    auto const server = static_cast<HttpServer*>(conn->server_param);
    switch (ev)
    {
    case MG_AUTH:
        return MG_TRUE;
    case MG_REQUEST:
        if (strcmp(conn->request_method, "GET") == 0)
            return server->get(conn);
        else if (strcmp(conn->request_method, "PUT") == 0)
            return server->put(conn);
        else if (strcmp(conn->request_method, "DELETE") == 0)
            return server->del(conn);
        else if (strcmp(conn->request_method, "POST") == 0)
            return server->post(conn);
        // no break
    default:
        return MG_FALSE;
    }
}

int HttpServer::put(mg_connection* conn)
{
    return MG_FALSE;
}

int HttpServer::get(mg_connection* conn)
{
    for (auto const& x : getRoutes_)
    {
        if (strncmp(conn->uri, x.first.c_str(), x.first.size()) == 0)
        {
            return x.second(conn);
        }
    }
    return MG_FALSE;
}

int HttpServer::post(mg_connection* conn)
{
    return MG_FALSE;
}

int HttpServer::del(mg_connection* conn)
{
    return MG_FALSE;
}

void HttpServer::run()
{
    if (running_.exchange(true) == false)
    {
        while (!stopped_)
        {
            mg_poll_server(server_, 1000);
        }
        running_ = false;
    }
}

void HttpServer::stop()
{
    stopped_ = true;
}
