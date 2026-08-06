//
//  xml.cpp
//  fetch-json
//
//  Created by Corey Ferguson on 7/14/26.
//

#include "xml.h"

namespace xml {
    // Non-Member Fields

    static const std::map<char, std::string> escape_map = {
        { '&', "&amp;"},
        { '<', "&lt;" },
        { '>', "&gt;" },
        { '\"', "&quot;" },
        { '\'', "&apos;" }
    };

    // Non-Member Functions

    std::string escape(const std::string value) {
        int   len = (int) value.length() + 1;
        char* str = new char[pow(len)];

        strcpy(str, value.c_str());

        int i = 0;

        while (i < len - 1) {
            auto it = escape_map.find(str[i]);

            if (it == escape_map.end())
                i++;
            else {
                str[i] = '&';

                // Reserve capacity, as required
                int newlen = len;

                while (newlen < len + (* it).second.length() - 1 && !is_pow(newlen, 2))
                    newlen++;

                if (newlen != len + (* it).second.length() - 1) {
                    char* tmp = new char[pow(newlen * 2)];

                    for (int j = 0; j < len; j++)
                        tmp[j] = str[j];

                    delete[] str;

                    str = tmp;
                }

                // Insert entity reference
                int j;

                for (j = 1; j < (* it).second.length(); j++) {
                    str[len] = (* it).second[j];

                    for (int k = len; k > i + j; k--)
                        std::swap(str[k], str[k - 1]);

                    len++;
                }

                i += j;
            }
        }

        return std::string(str);
    }

    element* parse(element* target, const std::string_view source, const int start, const int end) {
        auto handle_error = [](std::string what) {
            return error("Error: " + what);
        };

        int i = start;

        while (i < end) {
            if (isspace(source[i]))
                i++;
            // Step 1. Find tag open
            else if (source[i] == '<') {
                // <i...
                i++;

                int  j,
                      p = 1;
                bool escape = false;
                      
                // Step 2. Find tag close
                for (j = i; j < end; j++) {
                    // Escape element attributes
                    if (source[j] == '\"')
                        escape = !escape;
                    else if (!escape) {
                        if (source[j] == '<')
                            p++;
                        else if (source[j] == '>') {
                            // <i...j>
                            if (p == 1)
                                break;

                            p--;
                        }
                    }
                }

                if (j == end)
                    throw handle_error("Unexpected end of XML input");

                // Step 3. Check if element is self-closing
                int k;

                for (k = i; k < j; k++) {
                    if (source[k] == '\"')
                        escape = !escape;
                    else if (!escape && source[k] == '/')
                        break;
                }

                bool self_closing;

                if (k != j) {
                    if (k != j - 1)
                       throw handle_error("Unexpected token / in XML input");

                    self_closing = true;
                } else self_closing = false;

                // Parse element name and attributes
                std::vector<std::string_view> attributes = tokens(source.substr(i, k - i));

                // <i...>jk...
                k = ++j;

                if (attributes.empty())
                    attributes.push_back("");
                    // Ignore comments
                else if (attributes[0] == "!--") {
                    int l = 1;

                    while (l < attributes.size() && attributes[l] != "--")
                        l++;

                    if (l != attributes.size() - 1)
                        throw handle_error("Unexpected token -- in XML input");
                    // Parse document type declaration
                } else if (attributes[0] == "!DOCTYPE") {
                    if (attributes.size() <= 1)
                        throw handle_error("Unexpected end of XML input");

                    if (target->parent() != NULL && target->parent()->children().size() >= 2)
                        throw handle_error("Document type declaration must occur before content");
                    
                    target->dtd().name() = attributes[1];

                    for (int l = 2; l < attributes.size(); l++) {
                        std::vector<std::string> identifiers(attributes.begin() + 2, attributes.end());

                        target->dtd().identifiers(identifiers);
                    }
                    // Parse XML directive 
                } else if (attributes[0] == "?xml") {
                    if (attributes.size() == 1)
                        throw handle_error("Unexpected end of XML input");

                    std::unordered_map<std::string, std::string> xd;

                    for (int l = 1; l < attributes.size() - 1; l++) {
                        std::vector<std::string_view> attribute = split(attributes[l], "=");
                        
                        if (attribute.size() > 2)
                            throw handle_error("Unexpected token = in XML input");

                        xd[std::string(attribute[0])] = attribute.size() == 1 ? "" : std::string(attribute[1]);
                    }

                    std::vector<std::string_view> tokens = split(attributes[attributes.size() - 1], "?");

                    if (tokens.size() != 2)
                        throw handle_error("Unexpected end of XML input");

                    if (tokens[1].length())
                        throw handle_error("Unexpected token " + std::string((char[]) { tokens[1][0], '\0' }) + " in XML input");
                    
                    if (target->parent() != NULL && target->parent()->children().size())
                        throw handle_error("XML declaration must occur before DTD and content");

                    std::vector<std::string_view> attribute = split(tokens[0], "=");

                    if (attribute.size() > 2)
                        throw handle_error("Unexpected token = in XML input");

                    xd[std::string(attribute[0])] = attribute.size() == 1 ? "" : ::unescape(std::string(attribute[1]));

                    auto it = xd.find("version");

                    if (it != xd.end()) {
                        // Unescape single quotes
                        std::vector<std::string> version = split((* it).second, "'");

                        int l = 0;

                        while (l < version.size()) {
                            if (version[l].empty())
                                version.erase(version.begin() + l);
                            else l++;
                        }

                        target->xd().version(parse_number(join(version, "")));
                    }

                    it = xd.find("encoding");

                    if (it != xd.end())
                        target->xd().encoding() = (* it).second;

                    it = xd.find("standalone");

                    if (it != xd.end())
                        target->xd().standalone() = (* it).second == "yes" ? true : false;
                } else {
                    // Embed element
                    element* e = new element(std::string(attributes[0]));

                    target->add_child(e);

                    e->empty() = self_closing;

                    // Parse attributes
                    for (int l = 1; l < attributes.size(); l++) {
                        std::vector<std::string> attribute = copy(split(attributes[l], "="));
                        
                        merge(attribute, "=");

                        e->attributes()[attribute[0]] = attribute.size() == 1 ? "" : ::unescape(attribute[1]);
                    }

                    // Step 4. Find closing tag, if required
                    if (!self_closing) {
                        int l = -1;

                        for (; k < end; k++) {
                            if (source[k] == '\"')
                                escape = !escape;
                                // <i...>j...k<...
                            else if (!escape && source[k] == '<') {
                                int p = 1;

                                for (l = k + 1; l < end; l++) {
                                    if (source[l] == '\"')
                                        escape = !escape;
                                    else if (!escape) {
                                        if (source[l] == '<')
                                            p++;
                                        else if (source[l] == '>') {
                                            // <i...>j...k<...l>
                                            if (p == 1)
                                                break;

                                            p--;
                                        }
                                    }
                                }

                                if (l == end)
                                    throw handle_error("Unexpected end of XML input");

                                if (source[k + 1] == '/') {
                                    // <i...j>...<k/...l>
                                    if (source.substr(k + 2, l - k - 2) == attributes[0])
                                        break;
                                    else
                                        // <i...>j...</...l>k
                                        k = l;
                                }
                            }
                        }

                        if (k == end)
                            throw handle_error("Unexpected end of XML input");
                        
                        // Parse element content
                        j != k && parse(e, source, j, k);
                        
                        k = l + 1;
                    }
                }

                // Parse subsequent elements
                // k < end && parse(target, source, k, end);

                i = k;
            } else {
                if (source[i] == '>')
                    throw handle_error("Unexpected token > in XML input");

                // Embed text content
                element* e = new element();

                target->add_child(e);

                int j = i + 1;

                while (j < end && source[j] != '<')
                    j++;

                e->text(xml::unescape(std::string(source.substr(i, j - i))));

                i = j;
            }
        }

        return target;
    }

    std::string unescape(const std::string value) {
        int   len = (int) value.length();
        char* str = new char[len + 1];

        strcpy(str, value.c_str());

        // Shortest escape sequence is four characters in length
        for (int i = 0; i <= len - 4; i++) {
            for (const auto& [key, value2]: escape_map) {
                if (i <= value.length() - value2.length()) {
                    int j = 0;

                    while (j < value2.length() && str[i + j] == value2[j])
                        j++;
                    
                    if (j == value2.length()) {
                        str[i] = key;

                        for (int k = 1; k < value2.length(); k++) {
                            for (int l = i + 1; l < len; l++)
                                std::swap(str[l], str[l + 1]);

                            len--;
                        }
                        break;
                    }
                }
            }
        }

        return std::string(str);
    }

    // Constructors

    element::element() {
        this->_type = XML_TEXT_TYPE;
    }

    element::element(const std::string name) {
        this->_name = name;
        this->_type = XML_ELEMENT_TYPE;

        if (this->name().empty())
            this->xd().display() = true;
    }

    error::error(const std::string what) {
        this->_what = what;
    }

    element::~element() {
        for (element* child: this->_children)
            delete child;
    }

    // Member Functions

    void element::add_child(element* child) {
        if (this->type() == XML_ELEMENT_TYPE) {
           if (this->parent() == NULL && this->_children.size())
               throw error("Error: Root element can only have one child");
        }  else if (this->type() == XML_TEXT_TYPE) {
            if (this->text().length())
                throw error("Error: Text element cannot have children");

            this->_type = XML_ELEMENT_TYPE;
        }

        if (child->parent() != NULL)
            child->remove();

        child->_parent = this;

        this->_children.push_back(child);
    }

    std::unordered_map<std::string, std::string>& element::attributes() {
        return this->_attributes;
    }

    std::vector<element*> element::children() {
        return this->_children;
    }

    bool& element::preamble::display() {
        return this->_display;
    }

    class element::document_type& element::dtd() {
        return this->_dtd;
    }

    bool& element::empty() {
        return this->_empty;
    }

    std::string& element::preamble::encoding(){
        return this->_encoding;
    }

    element* element::find(const std::string name) {
        if (this->name() == name)
            return this;

        for (int i = 0; i < this->_children.size(); i++) {
            element* result = this->_children[i]->find(name);

            if (result != NULL)
                return result;
        }

        return NULL;
    }

    element* element::find(const std::string attr_key, const std::string attr_value) {
        auto it = this->attributes().find(attr_key);

        if (it == this->attributes().end() || (* it).second != attr_value) {
            for (int i = 0; i < this->_children.size(); i++) {
                element* result = this->_children[i]->find(attr_key, attr_value);

                if (result != NULL)
                    return result;
            }

            return NULL;
        }

        return this;
    }

    std::vector<element*> element::find_all(const std::string name) {
        std::vector<element*> target;
        
        return this->find_all(target, name);
    }

    std::vector<element*> element::find_all(const std::string attr_key, const std::string attr_value) {
        std::vector<element*> target;

        return this->find_all(target, attr_key, attr_value);
    }

    std::vector<element*> element::find_all(std::vector<element*>& target, const std::string name) {
        if (this->name() == name)
            target.push_back(this);

        for (int i = 0; i < this->_children.size(); i++) {
            element* result = this->_children[i]->find(name);

            if (result != NULL)
                target.push_back(result);
        }

        return target;
    }

    std::vector<element*> element::find_all(std::vector<element*>& target, const std::string attr_key, const std::string attr_value) {
        auto it = this->attributes().find(attr_key);

        if (it == this->attributes().end() || (* it).second != attr_value) {
            for (int i = 0; i < this->_children.size(); i++) {
                element* result = this->_children[i]->find(attr_key, attr_value);

                if (result != NULL)
                    target.push_back(result);
            }
        } else
            target.push_back(this);
        
        return target;
    }

    std::vector<std::string> element::document_type::identifiers() const {
        return this->_identifiers;
    }

    void element::document_type::identifiers(const std::vector<std::string> value) {
        if (this->name().empty())
            throw error("Error: DOCTYPE is not defined");

        if (value.empty() || (value[0] != "PUBLIC" && value[0] != "SYSTEM"))
            throw error("Error: First identifier must be PUBLIC or SYSTEM");

        if (value.size() == 1)
            throw error("Error: Expected identifier");
        
        this->_identifiers = value;
    }

    std::string element::name() const {
        return this->_name;
    }

    std::string& element::document_type::name() {
        return this->_name;
    }

    void element::name(const std::string name) {
        if (this->type() == XML_TEXT_TYPE && !this->text().empty())
            throw error("Error: Text element cannot have children");

        this->_name = name;
    }

    element* element::parent() {
        return this->_parent;
    }

    element* parse(std::string text) {
        if (text.empty())
            throw error("Unexpected end of XML input");

        element* e = new element("");

        e->xd().display() = false;

        return parse(e, text, 0, (int) text.length());
    }

    void element::remove() {
        int i = 0;

        while (i < this->parent()->children().size() && this->parent()->children()[i] != this)
            i++;

        this->parent()->_children.erase(this->parent()->_children.begin() + i);

        this->_parent = NULL;
    }

    bool& element::preamble::standalone() {
        return this->_standalone;
    }

    std::string element::str() {
        std::stack<element*> stack;
        std::string          result;

        stack.push(this);

        element* previous = NULL;
        int      indent = -1;

        this->parent() != NULL && indent++;

        while (!stack.empty()) {
            element* current = stack.top();

            switch (current->type()) {
                case XML_ELEMENT_TYPE: {
                    auto append_headers = [current, &result] {
                        if (current->name().empty()) {
                            if (current->parent() == NULL) {
                                if (current->xd().display() ||
                                    current->xd().version() != 1.0 ||
                                    current->xd().encoding() != "UTF-8" ||
                                    current->xd().standalone()) {
                                        result += "<?xml version=\"";
                                        result += truncate(current->xd().version(), 1);
                                        result += "\" encoding=\"";
                                        result += current->xd().encoding();
                                        result += "\"";

                                        if (current->xd().standalone())
                                            result += " standalone=\"yes\"";
                                            
                                        result += "?>\n";
                                    }

                                if (current->dtd().name().length()) {
                                    result += "<!DOCTYPE ";
                                    result += current->dtd().name();

                                    for (std::string identifier: current->dtd().identifiers()) {
                                        result += " ";
                                        result += identifier;
                                    }

                                    result += ">\n";
                                }
                            }

                            return true;
                        }

                        return false;
                    };

                    if (current->_children.empty()) {
                        if (!append_headers()) {
                            result += "<";
                            result += current->name();

                            for (const auto& [key, value]: current->attributes()) {
                                result += " ";
                                result += key;
                                
                                if (value.length()) {
                                    result += "=";
                                    result += ::escape(value);
                                }
                            }

                            if (current->empty()) {
                                if (current->attributes().size())
                                    result += " ";
                                
                                result += "/";
                            }

                            result += ">\n";
                        }

                        stack.pop();

                        previous = current;
                        indent--;
                    } else if (previous == current->_children.back()) {
                        if (current->name().length())
                            result.append((indent - 1) * 2, ' ');

                        if (current->name().length() && !current->empty()) {
                            result += "</";
                            result += current->name();
                            result += ">\n";
                        }

                        stack.pop();

                        previous = current;
                        indent--;
                    } else {
                        if (!append_headers()) {
                            result.append(indent * 2, ' ');

                            result += "<";
                            result += current->name();

                            for (const auto& [key, value]: current->attributes()) {
                                result += " ";
                                result += key;
                                
                                if (value.length()) {
                                    result += "=";
                                    result += ::escape(value);
                                }
                            }

                            if (current->empty()) {
                                if (current->attributes().size())
                                    result += " ";
                                
                                result += "/";
                            }

                            result += ">\n";
                        }

                        for (int i = (int) current->_children.size(); i > 0; i--)
                            stack.push(current->_children[i - 1]);

                        indent++;
                    }
                    break;
                } case XML_TEXT_TYPE: {
                    result.append(indent * 2, ' ');

                    result += xml::escape(current->text());
                    result += "\n";

                    stack.pop();

                    previous = current;
                    break;
                } default:
                    break;
            }
        }

        return result;
    }

    std::string element::text() const {
        return this->_text;
    }

    void element::text(const std::string value) {
        if (this->type() == XML_ELEMENT_TYPE) {
            if (this->_children.size())
                throw error("Error: Text element cannot have children");

            this->_type = XML_TEXT_TYPE;
        }

        this->_text = value;
    }

    enum element::type element::type() const {
        return this->_type;
    }

    double element::preamble::version() const {
        return this->_version;
    }

    void element::preamble::version(const double value) {
        if (value < 1)
            throw error("Error: version < 1.0");

        this->_version = value;
    }

    const char* error::what() const throw() {
        return this->_what.c_str();
    }

    element::preamble& element::xd() {
        return this->_xd;
    }
}
