//
//  xml.h
//  fetch-json
//
//  Created by Corey Ferguson on 7/14/26.
//

#ifndef xml_h
#define xml_h

#include "util.h"

namespace xml {
    // Typedef

    class error: public std::exception {
        // Member Fields

        std::string _what;
    public:
        // Constructors

        error(const std::string what);
    
        // Member Fields

        const char* what() const throw();
    };

    struct element {
        // Typedef

        enum type { XML_ELEMENT_TYPE, XML_TEXT_TYPE };

        class document_type {
            // Member Fields

            std::string              _name = "";
            std::vector<std::string> _identifiers;
        public:
            // Member Functions

            std::string&             name();

            // Return DTD identifiers
            std::vector<std::string> identifiers() const;
            
            // Assign DTD identifiers
            // Name must be non-empty, first identifier must be PUBLIC or SYSTEM
            void                     identifiers(std::vector<std::string> value);
        };

        class preamble {
            // Member Fields

            bool        _display = false;
            std::string _encoding = "UTF-8";
            bool        _standalone = false;
            double      _version = 1.0;
        public:
            // Member Functions

            bool&        display();

            std::string& encoding();

            bool&        standalone();

            double       version() const;

            // Value must be >= 1.0
            void         version(const double value);
        };
        
        // Constructors

        // Instantiate element of type XML_TEXT_TYPE
        element();

        // Instantiate element of type XML_ELEMENT_TYPE; leave name empty for root
        element(const std::string name);

        ~element();

        // Member Functions

        // Add child to element; removes child from parent, if non-null
        void                                          add_child(element* child);
        
        std::unordered_map<std::string, std::string>& attributes();

        // Return a read-only copy of element's children
        std::vector<element*>                         children();

        // Return document type directive
        document_type&                                dtd();

        bool&                                         empty();

        // Return first child of name
        element*                                      find(const std::string name);

        // Return first child with attribute
        element*                                      find(const std::string attr_key, const std::string attr_value);

        // Return all children of name
        std::vector<element*>                         find_all(const std::string name);

        // Return all children with attribute
        std::vector<element*>                         find_all(const std::string attr_key, const std::string attr_value);

        // Return element name
        std::string                                   name() const;

        // Assign element name; element must have empty text
        void                                          name(const std::string name);

        // Return NULL if root; otherwise, return parent
        element*                                      parent();

        // Remove oneself from parent
        void                                          remove();

        std::string                                   str();

        // Return element text
        std::string                                   text() const;

        // Assign element text; element must not have children
        void                                          text(const std::string value);

        // Return read-only element type; determined by name and children
        enum type                                     type() const;

        // Return XML directive
        preamble&                                     xd();
    private:

        // Member Fields

        std::unordered_map<std::string, std::string> _attributes;
        std::vector<element*>                        _children;
        document_type                                _dtd;
        bool                                         _empty = false;
        std::string                                  _name = "";
        element*                                     _parent = NULL;
        std::string                                  _text = "";
        enum type                                    _type;
        preamble                                     _xd;

        // Member Functions

        // Recursively return all children of name
        std::vector<element*> find_all(std::vector<element*>& target, const std::string name);

        // Recursively return all children with attribute
        std::vector<element*> find_all(std::vector<element*>& target, const std::string attr_key, const std::string attr_value);
    };

    // Non-Member Functions

    // Replace reserved characters with their entity references
    std::string escape(const std::string value);

    element*    parse(const std::string text);

    // Replace entity references with their reserved characters
    std::string unescape(const std::string value);
}

#endif /* xml_h */
