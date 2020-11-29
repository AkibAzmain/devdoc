#include <docview.hpp>

#include <map>
#include <libxml++/parsers/domparser.h>
#include <libxml++/nodes/node.h>
#include <libxml++/nodes/element.h>

class devdoc : public docview::extension
{
private:

    // Shared pointer to all parsers used to parse devhelp files
    std::vector<std::shared_ptr<xmlpp::DomParser>> parsers;

    // Map of element nodes of libxml++, mapped with document nodes
    std::map<const docview::doc_tree_node*, xmlpp::Element*> nodes;

    // An array of all root nodes created
    std::vector<const docview::doc_tree_node*> root_nodes;

    // Builds an document tree from an chapter
    void build_chapters_tree(docview::doc_tree_node* node, xmlpp::Node* source)
    {

        // Check if it is a valid chapter or subject
        auto element = dynamic_cast<xmlpp::Element*>(source);
        if (element && element->get_name() == "sub")
        {

            // Create new node and setup it
            docview::doc_tree_node* child = new docview::doc_tree_node;
            child->title = element->get_attribute_value("name");
            child->parent = node;

            // Do the same recursively for all children
            for (auto& subject : element->get_children())
                build_chapters_tree(child, subject);

            // Add it to the tree
            node->children.push_back(child);
            nodes.insert(std::make_pair(child, element));
        }
    }

    // Delete node with all of it's children
    void delete_node(const docview::doc_tree_node* node)
    {

        // Delete all children
        for (auto& child : node->children)
            delete_node(child);

        // Delete the node
        delete node;
    }

public:

    // The destructor, releases memory
    ~devdoc()
    {
        for (auto& node : root_nodes)
        {
            delete_node(node);
        }
    }

    // The applicability level, which is small
    applicability_level get_applicability_level() noexcept
    {
        return applicability_level::small;
    }

    // Generates a document tree
    const docview::doc_tree_node* get_doc_tree(std::filesystem::path path) noexcept
    {

        // Make sure it's a valid path
        if (
            !std::filesystem::is_directory(path) ||
            !std::filesystem::exists(
                path / (std::string(path.filename()) + ".devhelp2")
            ) ||
            !std::filesystem::is_regular_file(
                path / (std::string(path.filename()) + ".devhelp2")
            )
        )
        {
            return nullptr;
        }

        // Wrapped with try-catch block, return nullptr on any unhandled exception
        try
        {

            // Create new parser
            parsers.push_back(std::make_shared<xmlpp::DomParser>());
            auto parser = parsers[parsers.size() - 1];

            // Parse the file
            parser->parse_file(Glib::ustring(path / (std::string(path.filename()) + ".devhelp2")));

            // Get the root element and check for validity, return nullptr on failure
            auto xml_root = parser->get_document()->get_root_node();
            if (!xml_root || xml_root->get_name() != "book") return nullptr;

            // Create new root node
            docview::doc_tree_node* root = new docview::doc_tree_node;
            root->title = xml_root->get_attribute_value("title");
            root->parent = nullptr;

            // Parse all chapters
            auto chapters = xml_root->get_first_child("chapters");
            if (chapters)
            {
                for (auto& subject : chapters->get_children())
                    build_chapters_tree(root, subject);
            }

            // Try to parse any extra links
            auto functions = xml_root->get_first_child("functions");
            if (functions)
            {

                // Create a parent node for all extra nodes
                docview::doc_tree_node* parent = new docview::doc_tree_node;
                parent->title = "More (keywords, functions...)";
                parent->parent = root;

                // Pointers to parent nodes of single keyword types
                std::map<Glib::ustring, docview::doc_tree_node*> known_keyword_types;

                // Parse any extra links
                for (auto& function : functions->get_children())
                {

                    // Cast to element node, and check for validity
                    auto element = dynamic_cast<xmlpp::Element*>(function);
                    if (!element || function->get_name() != "keyword")
                        continue;

                    // Parent of new node
                    docview::doc_tree_node* keyword_type_parent;

                    // Get address of parent, create if doesn't exist
                    try
                    {
                        keyword_type_parent = known_keyword_types.at(element->get_attribute_value("type"));
                    }
                    catch(const std::out_of_range&)
                    {

                        // Create new parent node and set it up
                        keyword_type_parent = new docview::doc_tree_node;
                        keyword_type_parent->title = element->get_attribute_value("type");
                        keyword_type_parent->parent = parent;
                        parent->children.push_back(keyword_type_parent);
                        known_keyword_types.insert(std::make_pair(
                            element->get_attribute_value("type"),
                            keyword_type_parent
                        ));
                    }

                    // Create new node and set it up
                    docview::doc_tree_node* node = new docview::doc_tree_node;
                    node->title = element->get_attribute_value("name");
                    node->parent = keyword_type_parent;
                    keyword_type_parent->children.push_back(node);
                    nodes.insert(std::make_pair(node, element));
                }

                // If there is no extra nodes, delete this unnecessary one too
                if (parent->children.size() == 0)
                    delete parent;

                // Otherwise, add this to the document tree
                else
                    root->children.push_back(parent);
            }

            // Save it for further requests
            nodes.insert(std::make_pair(root, xml_root));
            root_nodes.push_back(root);

            // Return it
            return root;
        }
        catch(const std::exception&)
        {

            // An exception occurred, return nullptr
            return nullptr;
        }
    }

    // Returns the document of a node
    std::pair<std::string, bool> get_doc(const docview::doc_tree_node* node) noexcept
    {

        // Get the root
        const docview::doc_tree_node* root = node;
        while (root->parent)
            root = root->parent;

        // Return the URI
        return std::make_pair(
            "file://" + nodes[root]->get_attribute_value("base") +
            "/" + nodes[node]->get_attribute_value("link"),
            true
        );
    }
};

extern "C"
{

    // The extension object
    devdoc extension_object;
}
