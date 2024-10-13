#include <iostream>
#include <string>
#include <typeindex>
#include <vector>
#include <variant>
#include <unordered_map>
#include "archive.h"

namespace fast {

void SqlArchive::load_data(std::unordered_map<std::string, std::string> result_data) {
   this->result_data = result_data;
}

void SqlArchive::add(std::string name, int64_t* data) {
    this->map[name] = data;
}
void SqlArchive::add(std::string name, double* data) {
    this->map[name] = data;
}
void SqlArchive::add(std::string name, std::string* data) {
    this->map[name] = data;
}
void SqlArchive::add(std::string name, bool* data) {
    this->map[name] = data;
}
void SqlArchive::add(std::string name, Archivable* data) {
    this->map[name] = data;
}

void SqlArchive::add(std::string name, std::vector<Archivable>* data) {
    this->map[name] = data;
}

void SqlArchive::deserialise() {
   // loop through data stored in result_data
   for (const auto& [key, value] : result_data) {
      // translate string data into type
      if (map.contains(key)) {
         std::stringstream error {};

         switch (map[key].index()) {
         case TypeId::Int64:
            try {
               int64_t* ptr = std::get<TypeId::Int64>(map[key]);
               *ptr = std::stol(value);
               //*(std::get<TypeId::Int64>(map[key])) = std::stol(value);

            } catch (std::exception& e) {
               error << "error, field " << key << " not convertible to " 
                     << TYPE_ID_NAMES[TypeId::Int64]
                     << "\n" << e.what();
               throw std::runtime_error(error.str());
            }
				break;
         case TypeId::Float64:
            try {
               double* ptr = std::get<TypeId::Float64>(map[key]);
               *ptr = std::stod(value);
            } catch (std::exception& e) {
               error << "error, field " << key << " not convertible to " 
                     << TYPE_ID_NAMES[TypeId::Float64]
                     << "\n" << e.what();
               throw std::runtime_error(error.str());
            }
				break;
         case TypeId::String:
            try {
               std::string* ptr = std::get<TypeId::String>(map[key]);
               *ptr = value;
            } catch (std::exception& e) {
               error << "error, field " << key << " not convertible to " 
                     << TYPE_ID_NAMES[TypeId::String]
                     << "\n" << e.what();
               throw std::runtime_error(error.str());
            }
				break;
         case TypeId::Bool:
            try {
               bool* ptr = std::get<TypeId::Bool>(map[key]);
               *ptr = value == "true" ? true : false;
            } catch (std::exception& e) {
               error << "error, field " << key << " not convertible to " 
                     << TYPE_ID_NAMES[TypeId::Bool]
                     << "\n" << e.what();
               throw std::runtime_error(error.str());
            }
				break;

         // this one is going to change, for now we assume the json will all be
         // in a big hashmap of strings, so "key": "{"key_one": 12}"

         case TypeId::Object:
            try {
               Archivable* ptr = std::get<TypeId::Object>(map[key]);
               SqlArchive local_archive;
               ptr->load_metadata(local_archive);

               // call some parse json function then load the archive with the
               // resulting data

               // recurse through this object and assign the json data;
               local_archive.deserialise();

            } catch (std::exception& e) {
               error << "error, field " << key << " not convertible to " 
                     << TYPE_ID_NAMES[TypeId::Object]
                     << "\n" << e.what();
               throw std::runtime_error(error.str());
            }
				break;
         case TypeId::Array:
            try {
               // not implemented yet
               //*(std::get<TypeId::Int64>(map[key])) = std::stod(value);
            } catch (std::exception& e) {
               error << "error, field " << key << " not convertible to " 
                     << TYPE_ID_NAMES[TypeId::Array] 
                     << "\n" << e.what();
               throw std::runtime_error(error.str());
            }
				break;
         }
      } else {
         throw std::runtime_error("error, value in json data not found in our type");
      }
   }
}

void SqlArchive::serialise() {}

} // namespace fast


struct Data : fast::Archivable {
   int64_t id;
   std::string name;
   std::string address;

   void load_metadata(fast::Archive& arc) {
      arc.add("id", &id);
      arc.add("name", &name);
      arc.add("address", &address);
   }

   friend std::ostream& operator<<(std::ostream& os, Data& d) {
      return os << "{" << d.id << ", " << d.name << ", " << d.address << "}";
   }
};

int main() {
   Data d;

   fast::SqlArchive arc;
   arc.load_data({
      {"id", "1234"},
      {"name", "kai gibson"},
      {"address", "224 Cardigan St"},
   });

   d.load_metadata(arc);

   arc.deserialise();

   std::cout << d << "\n";
}
