#include "object.h"
#include <raptor2.h>
#include <string>
#include <vector>
#include <functional>
#include <iostream>

using namespace std;
using namespace sbol;


SBOLObject::~SBOLObject()
{
}


void SBOLObject::close()
{
    delete this;
};

sbol_type SBOLObject::getTypeURI() 
{
	return type;
};

std::string SBOLObject::getClassName(string type)
{
    std::size_t uri_subordinate_pos = type.find("#") + 1;
    if (uri_subordinate_pos != std::string::npos)
    {
        std::string sbol_class = type.substr(uri_subordinate_pos, type.size() - uri_subordinate_pos);
        return sbol_class;
    }
    else
        return type;
};


int SBOLObject::compare(SBOLObject* comparand)
{
    int IS_EQUAL = 1;
    if (type.compare(comparand->type) != 0)
    {
        std::cout << identity.get() << " does not match type of " << comparand->type << endl;
        return 0;
    };

    std::string l_id;
    std::string r_id;
    std::map < std::string, std::vector<std::string> >::iterator i_lp;  // iterator for left-hand side
    std::map < std::string, std::vector<std::string> >::iterator i_rp;  // iterator for right-hand side
    std::map < std::string, std::vector<std::string> >::iterator i_end;  // stop iteration

    // The longer property store is assigned to left-hand side for side-by-side comparison. Property keys are assumed alphabetically sorted since they are based on std::map
    if (properties.size() >= comparand->properties.size())
    {
        l_id = identity.get();
        r_id = comparand->identity.get();
        i_lp = properties.begin();
        i_rp = comparand->properties.begin();
        i_end = properties.end();
    }
    else
    {
        l_id = comparand->identity.get();
        r_id = identity.get();
        i_lp = comparand->properties.begin();
        i_rp = properties.begin();
        i_end = comparand->properties.end();

    }
    while (i_lp != i_end )
    {
        std::string l_key = i_lp->first;
        std::string r_key = i_rp->first;
        // In case one object has a property (ie, extension annotation) that another object does not
        if(l_key.compare(r_key) != 0)
        {
            cout << r_id << "::" << parsePropertyName(r_key) << " not found in " << l_id << endl;
            ++i_lp;
            IS_EQUAL = 0;
        }
        // If the property's match, then iterate through property values and compare them
        else
        {
            // Copy and sort the property store, so we can compare them side by side
            std::vector < std::string > l_store(i_lp->second);
            std::vector < std::string > r_store(i_rp->second);
            std::sort(l_store.begin(), l_store.end());
            std::sort(r_store.begin(), r_store.end());
            if (!std::equal(l_store.begin(), l_store.end(), r_store.begin()))
            {
                cout << "Values " << " in " << l_id << "::" << parsePropertyName(l_key) << " not equal to values in " << r_id << "::" << parsePropertyName(r_key) << endl;
                IS_EQUAL = 0;
            }
            ++i_lp;
            ++i_rp;
        }
    }

    /// @TODO this may fail if there are extension properties resulting in owned_objects of differing lengths in the comparand objects
    for (auto i_p = owned_objects.begin(); i_p != owned_objects.end(); ++i_p)
    {
        string p = i_p->first;
        
        SBOLObject* l;
        SBOLObject* r;
        if (this->owned_objects[p].size() >= comparand->owned_objects[p].size())
        {
            l = this;
            r = (SBOLObject*)comparand;
        }
        else
        {
            l = (SBOLObject*)comparand;
            r = this;
        }

        vector<SBOLObject*>& l_object_store = l->owned_objects[p];
        vector<SBOLObject*>& r_object_store = r->owned_objects[p];
        
        map<string, SBOLObject*> l_store_map;
        map<string, SBOLObject*> r_store_map;
        for (auto i_obj = l_object_store.begin(); i_obj != l_object_store.end(); ++i_obj)
        {
            SBOLObject* obj = *i_obj;
            l_store_map[obj->identity.get()] = obj;
        }
        for (auto i_obj = r_object_store.begin(); i_obj != r_object_store.end(); ++i_obj)
        {
            SBOLObject* obj = *i_obj;
            r_store_map[obj->identity.get()] = obj;
        }
        map<string, SBOLObject*>::iterator l_p = l_store_map.begin();
        map<string, SBOLObject*>::iterator r_p = r_store_map.begin();
        while (l_p != l_store_map.end())
        {
            string l_id = l_p->first;
            string r_id = r_p->first;
            if (l_id.compare(r_id) == 0)
            {
                SBOLObject* l_obj = l_store_map[l_id];
                SBOLObject* r_obj = r_store_map[r_id];
                if (!l_obj->compare(r_obj))  // Recurse into child objects and compare
                    IS_EQUAL = 0;
                ++l_p;
                ++r_p;
            }
            else
            {
                cout << "Child object " << l_id << " not in " << parsePropertyName(p) << " property of " << r->identity.get() << endl;
                IS_EQUAL = 0;
                ++l_p;
            }
        }
    }
    if (IS_EQUAL)
        return 1;
    else
        return 0;
};

int SBOLObject::find(string uri)
{
    if (identity.get() == uri)
        return 1;
    for (auto i_store = owned_objects.begin(); i_store != owned_objects.end(); ++i_store)
    {
        vector<SBOLObject*>& store = i_store->second;
        for (auto i_obj = store.begin(); i_obj != store.end(); ++i_obj)
        {
            SBOLObject& obj = **i_obj;
            if (obj.find(uri))
                return 1;
        }
    }
    return 0;
};


std::string SBOLObject::getPropertyValue(std::string property_uri)
{
    if (properties.find(property_uri) != properties.end())
    {
        std::string property_value = properties[property_uri][0];
        property_value = property_value.substr(1, property_value.length() - 2);
        return property_value;
    }
    else throw SBOLError(SBOL_ERROR_NOT_FOUND, property_uri + " not contained in this object.");
};


std::vector < std::string > SBOLObject::getPropertyValues(std::string property_uri)
{
    if (properties.find(property_uri) != properties.end())
    {
        std::vector < std::string > property_values = properties[property_uri];
        for (auto i_val = 0; i_val < property_values.size(); ++i_val)
        {
            std::string val = property_values[i_val];
            property_values[i_val] = val.substr(1, val.length() - 2);
        }
        return property_values;
    }
    else throw SBOLError(SBOL_ERROR_NOT_FOUND, property_uri + " not contained in this object.");
};


std::vector < std::string > SBOLObject::getProperties()
{
    std::vector < std::string > property_uris;
    for (auto i_p = properties.begin(); i_p != properties.end(); ++i_p)
        property_uris.push_back(i_p->first);
    return property_uris;
};

