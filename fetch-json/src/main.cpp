//
//  main.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"
#include "xml.h"

using namespace fetch;
using namespace std;

// Non-Member Fields

http_client http;

// Non-Member Functions

logging parse_logging(const std::string value) {
    int index = ((map<string, int>) {
        { "none", 1 },
        { "some", 2 },
        { "more", 3 },
    })[tolowerstr(value)] - 1;

    if (index == -1) {
        cout << "Option '" + value + "' not recognized\n";

        return LOG_SOME;
    }

    return static_cast<enum logging>(index);
}

void set_logging(map<string, string> options) {
    auto it = options.find("-l");

    if (it == options.end())
        it = options.find("--log");

    http.set_logging(
        it == options.end() ?
            LOG_SOME :
            parse_logging((* it).second)
    );
}

int main(int argc, const char* argv[]) {
    map<string, string> opts = options(argc, argv);

    set_logging(opts);

    http.timeout() = 60;

    header::map headers;

    try {
        cout << "Fetching all vehicle makes from the NHTSA...\n";

        class url url("https://vpic.nhtsa.dot.gov/api/vehicles/getallmakes");

        url.params()["format"] = string("xml");

        auto response = http.get(headers, url.str());

        std::chrono::time_point start = std::chrono::steady_clock::now();

        auto xml = xml::parse(response.text());

        cout << xml->find("Results")->str() << endl;

        cout << (std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
            .count() * 1000) << " ms\n";
    } catch (fetch::error& e) {
        throw e;
    }
}
