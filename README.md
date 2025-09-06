# Fetch-JSON SDK for C++

Thank you for choosing the <i>Fetch-JSON SDK for C++</i>.

It combines two existing SDKs <i>fetch</i> and <i>json</i>, enabling C++ developers to natively handle HTTP requests end-to-end for low-level integration.

# Setup
1. Open the project in VS Code and navigate to the integrated terminal
2. Ensure you have Node v18+ installed by running `node -v`
3. Run `npm install`
4. Open the project in Xcode
5. Click Command + B on the keyboard to build the project for the first time

## Build

1. Click Command + , to open Settings and click the arrow (->) under <i>Derived Data</i>
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
./fetch-json
```

## Reference

```
string              url = "http://localhost:8080/greeting";
string              method = "POST";
auto                body = new object((vector<object*>) {
    new object("firstName", encode("Corey")),
});
map<string, string> headers = {{ "content-type", "application/json" }};

try {
    auto response = request(headers, url, method, stringify(body));

    cout << stringify(response.json()) << endl;
} catch (fetch::error& e) {
    throw e;
}
```
