//
//  main.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "fetch.h"
#include "json.h"

using namespace fetch;
using namespace json;
using namespace std;

map<string, string> headers = {{ "content-type", "application/json" }};

int main(int argc, const char * argv[]) {
    string url = "http://localhost:8081/greeting";
    string method = "POST";
    auto   body = new object((vector<object*>) {
        new object("firstName", encode("Corey")),
        // new object("lastName", encode("Ferguson"))
    });

    try {
        auto response = request(headers, url, method, stringify(body));

        cout << stringify(response.json()) << endl;
    } catch (fetch::error& e) {
        throw e;
    }
}
