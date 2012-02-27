/**
 * @file ctxml.cpp
 */
#define CANTERA_USE_INTERNAL
#include "ctxml.h"

// Cantera includes
#include "cantera/base/ctml.h"
#include "Cabinet.h"
#include "Storage.h"

#include <string.h>

using namespace std;
using namespace Cantera;
using namespace ctml;

// Assign storage for the static member of the Templated Cabinet class
// class Cabinet<XML_Node>;

typedef Cabinet<XML_Node, false> XmlCabinet;
template<> XmlCabinet* XmlCabinet::__storage = 0;

extern "C" {

    int DLL_EXPORT xml_new(const char* name = 0)
    {
        XML_Node* x;
        if (!name) {
            x = new XML_Node;
        } else {
            x = new XML_Node(name);
        }
        return XmlCabinet::add(x);
    }

    int DLL_EXPORT xml_get_XML_File(const char* file, int debug)
    {
        try {
            XML_Node* x = get_XML_File(std::string(file), debug);
            return XmlCabinet::add(x);
        } catch (CanteraError) {
            return -1;
        }
    }

    int DLL_EXPORT xml_clear()
    {
        try {
            XmlCabinet::clear();
            close_XML_File("all");
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int DLL_EXPORT xml_del(int i)
    {
        XmlCabinet::del(i);
        return 0;
    }

    int DLL_EXPORT xml_removeChild(int i, int j)
    {
        XmlCabinet::item(i).removeChild(&XmlCabinet::item(j));
        return 0;
    }

    int DLL_EXPORT xml_copy(int i)
    {
        return XmlCabinet::newCopy(i);
    }

    int DLL_EXPORT xml_assign(int i, int j)
    {
        return XmlCabinet::assign(i,j);
    }

    int DLL_EXPORT xml_build(int i, const char* file)
    {
        try {
            writelog("WARNING: xml_build called. Use get_XML_File instead.");
            string path = findInputFile(string(file));
            ifstream f(path.c_str());
            if (!f) {
                throw CanteraError("xml_build",
                                   "file "+string(file)+" not found.");
            }
            XmlCabinet::item(i).build(f);
            f.close();
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }

    int DLL_EXPORT xml_preprocess_and_build(int i, const char* file, int debug)
    {
        try {
            get_CTML_Tree(&XmlCabinet::item(i), string(file), debug);
            return 0;
        } catch (CanteraError) {
            return -1;
        }
    }



    int DLL_EXPORT xml_attrib(int i, const char* key, char* value)
    {
        try {
            string ky = string(key);
            XML_Node& node = XmlCabinet::item(i);
            if (node.hasAttrib(ky)) {
                string v = node[ky];
                strncpy(value, v.c_str(), 80);
            } else
                throw CanteraError("xml_attrib","node "
                                   " has no attribute '"+ky+"'");
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_addAttrib(int i, const char* key, const char* value)
    {
        try {
            string ky = string(key);
            string val = string(value);
            XML_Node& node = XmlCabinet::item(i);
            node.addAttribute(ky, val);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_addComment(int i, const char* comment)
    {
        try {
            string c = string(comment);
            XML_Node& node = XmlCabinet::item(i);
            node.addComment(c);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_tag(int i, char* tag)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            const string v = node.name();
            strncpy(tag, v.c_str(), 80);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_value(int i, char* value)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            const string v = node.value();
            strncpy(value, v.c_str(), 80);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_child(int i, const char* loc)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            XML_Node& c = node.child(string(loc));
            return XmlCabinet::add(&c);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_child_bynumber(int i, int m)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            XML_Node& c = node.child(m);
            return XmlCabinet::add(&c);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_findID(int i, const char* id)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            XML_Node* c = node.findID(string(id));
            if (c) {
                return XmlCabinet::add(c);
            } else {
                throw CanteraError("xml_find_id","id not found: "+string(id));
            }
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_findByName(int i, const char* nm)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            XML_Node* c = node.findByName(string(nm));
            if (c) {
                return XmlCabinet::add(c);
            } else
                throw CanteraError("xml_findByName","name "+string(nm)
                                   +" not found");
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_nChildren(int i)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            return (int) node.nChildren();
        } catch (CanteraError) {
            return -1;
        }
    }

    int DLL_EXPORT xml_addChild(int i, const char* name, const char* value)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            XML_Node& c = node.addChild(string(name),string(value));
            return XmlCabinet::add(&c);
        } catch (CanteraError) {
            showErrors(cout);
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_addChildNode(int i, int j)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            XML_Node& chld = XmlCabinet::item(j);
            XML_Node& c = node.addChild(chld);
            return XmlCabinet::add(&c);
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT xml_write(int i, const char* file)
    {
        try {
            ofstream f(file);
            if (f) {
                XML_Node& node = XmlCabinet::item(i);
                node.write(f);
            } else {
                throw CanteraError("xml_write",
                                   "file "+string(file)+" not found.");
            }
            return 0;
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

    int DLL_EXPORT ctml_getFloatArray(int i, size_t n, doublereal* data, int iconvert)
    {
        try {
            XML_Node& node = XmlCabinet::item(i);
            vector_fp v;
            bool conv = false;
            if (iconvert > 0) {
                conv = true;
            }
            getFloatArray(node, v, conv);
            size_t nv = v.size();

            // array not big enough
            if (n < nv) {
                throw CanteraError("ctml_getFloatArray",
                                   "array must be dimensioned at least "+int2str(int(nv)));
            }

            for (size_t i = 0; i < nv; i++) {
                data[i] = v[i];
            }
        } catch (CanteraError) {
            return -1;
        }
        return 0;
    }

}
