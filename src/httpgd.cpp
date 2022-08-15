
#include <cpp11/function.hpp>
#include <cpp11/strings.hpp>
#include <cpp11/list.hpp>
#include <cpp11/integers.hpp>
#include <cpp11/logicals.hpp>
#include <cpp11/data_frame.hpp>
#include <cpp11/as.hpp>
#include <cpp11/raws.hpp>

//#include <R_ext/GraphicsEngine.h>

#include <vector>
#include <string>

#include "unigd_impl.h"

#include "httpgd_rng.h"
#include "httpgd_version.h"

#include <boost/optional.hpp>

#include "httpgd_webserver.h"

#include "debug_print.h"

namespace httpgd
{

    // returns system path to {package}/inst/www/{filename}
    std::string get_wwwpath(const std::string &filename)
    {
        using namespace cpp11::literals;

        auto sys_file = cpp11::package("base")["system.file"];

        cpp11::strings res(sys_file("www", filename,
                                          "package"_nm = "httpgd"));
        return res[0];
    }

    // --------------------------------------

} // namespace httpgd

[[cpp11::register]]
bool httpgd_(int devnum, std::string host, int port, bool cors, std::string token, 
             bool silent)
{
    bool recording = true;
    bool use_token = token.length();

    std::string wwwpath(httpgd::get_wwwpath(""));

    const httpgd::web::HttpgdServerConfig conf{host,
         port,
         wwwpath,
         cors,
         use_token,
         token,
         recording,
         silent,
         httpgd::rng::uuid()};

    return (new httpgd::web::WebServer(conf))->attach(devnum);
}

[[cpp11::register]]
cpp11::list httpgd_details_(int devnum)
{
    if (httpgd::ugd::api == nullptr) {
        cpp11::stop("unigd not initialized.");
    }
    auto *client = httpgd::ugd::api->device_get(devnum, httpgd::ugd::httpgd_client_id);
    if (!client) {
        cpp11::stop("Device is not a unigd device with attached httpgd client.");
    }

    auto *server = static_cast<httpgd::web::WebServer *>(client);
    const auto svr_config = server->get_config();
        
    using namespace cpp11::literals;
    return cpp11::writable::list{
        "host"_nm = svr_config.host.c_str(),
        "port"_nm = server->port(),
        "token"_nm = svr_config.token.c_str()};
        return cpp11::writable::list{};
}

[[cpp11::register]]
std::string httpgd_random_token_(int len)
{
    if (len < 0)
    {
        cpp11::stop("Length needs to be 0 or higher.");
    }
    return httpgd::rng::token(len);
}