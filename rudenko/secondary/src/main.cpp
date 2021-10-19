#include <chrono>
#include <cstdio>
#include <string>
#include <string>
#include <unordered_map>
#include "httplib.h"
#include "json.h"

using namespace httplib;
using namespace std;
using namespace nlohmann;

json storage = json::array();

int main(int argc, char *argv[]) {
    Server svr;
    if (!svr.is_valid()) {
        printf("server has an error...\n");
        return -1;
    } else {
        cout << "Server has been started!" << endl;
    }

    svr.Get("/", [](const Request& req, Response &res) {
        try {
            res.status = 200;
            res.set_content(storage.dump(), "application/json");
        } catch(exception& error) {
            res.status = 500;
        }
    });

    svr.Post("/", [](const Request& req, Response &res) {
        try {
            auto message = json::parse(req.body);
            storage.push_back(message);
            cout << "Received a message:" << endl;
            cout << message.dump() << endl;
        } catch(exception& error) {
            res.status = 500;
        }
    });

    svr.Get("/ping",
            [&](const Request & /*req*/, Response & res) { res.status = 200; });

    svr.Post("/shutdown",
            [&](const Request & /*req*/, Response & /*res*/) { svr.stop(); });

    svr.listen("127.0.0.1", 7777);

    return 0;
}
