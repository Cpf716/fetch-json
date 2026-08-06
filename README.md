# Fetch++

Thanks for checking out <i>fetch++</i>!

It combines three powerful SDKs, <i>fetch</i>, <i>json</i>, and <i>xml</i>, equipping C++ developers with a powerful native HTTP client for integration with most modern APIs.

# Setup
1. Open the project in VS Code and navigate to the integrated terminal
2. Ensure you have Node v18+ installed by running `node -v`
3. Run `npm install`
4. Open the project in Xcode
5. Click `Command + B` on the keyboard to build the project for the first time

## Build
1. Click `Command + ,` to open Settings, click <i>Locations</i> in the sidebar, and click the arrow (->) under <i>Derived Data</i>
2. A new Finder window will open at Xcode’s <i>Derived Data</i> directory; find your project's name and expand Build > Product > Debug
3. Launch Terminal and type `cd` + Space
4. Navigate back to Finder, click and drag the <i>Debug</i> folder into Terminal, and click Enter
5. Click Command + N to open a new window or Command + T to open a new tab
6. Type `cd` + Space once more, drag your project's root directory into the new Terminal session, and click Enter
7. In the same Terminal session, execute the following command:<br><br>
<i>* You will have to execute the command every time you modify the C++ code</i>
```
xcodebuild -project fetch-json.xcodeproj -scheme fetch-json
```
8. Navigate back to Xcode and click Command + M to minimize the window

## Run
### Test Server

1. Navigate to VS Code's integrated terminal
2. Execute the following command:
```
node test-server/src/index.js
```

### Command-Line Application
1. Navigate to the first Terminal window
2. Execute the following command:
```
./fetch-json [-l | --log] # none, some, or more
```

The test server is for your convenience getting started, however Fetch++ is feature-rich, including robust transport layer security, so you can access almost any resource on the internet.

## Reference

By default, the http_client executes synchronously. Request headers are mutated by the fetch API, therefore they must be passed by reference. The API accepts string arguments, which can be derived from serialized JSON or XML data.

```
fetch::http_client http;
fetch::header::map headers;

std::string url = "http://localhost:8080/api/greeting";

try {
    http.options(headers, url);

    unique_ptr<json::object> body = make_unique<json::object>((std::vector<json::object*>) {
        new json::object("firstName", escape("fetch++"))
    });
    
    auto response = http.post(headers, url, body.get());
    auto json = json::parse(response.text());

    std::cout << unescape(json->value()) << std::endl;
} catch (fetch::error& e) {
    throw e;
}
```
