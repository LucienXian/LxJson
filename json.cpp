#include "json.hpp"
#include <cstdio>
#include <stdexcept> //for runtime error

namespace lxjson{

using std::string;
using std::vector;
using std::map;

static const Json json_null;

//todo:
static void serialize(std::nullptr_t, string &out){
    out+="null";
}
static void serialize(int values, string &out){
    char c[32];
    snprintf(c, sizeof(c), "%d", values);
    out += c;
}
static void serialize(double values, string &out){
    char c[32];
    snprintf(c, sizeof(c), "%.17g", values);
    out += c;
}
static void serialize(bool values, string &out){
    out += values? "true":"false";
}
static void serialize(const string &values, string &out){}
static void serialize(const Json::array &values, string &out){}
static void serialize(const Json::object &values, string &out){}

//JsonValue
double JsonValue::double_value() const               {throw std::runtime_error("not a number!");}
int JsonValue::int_value() const                     {throw std::runtime_error("not a number!");}
const string &JsonValue::string_value() const        {throw std::runtime_error("not a string!");}
bool JsonValue::bool_value() const                   {throw std::runtime_error("not a boolean!");}
const Json::array &JsonValue::array_value() const    {throw std::runtime_error("not a array!");}
const Json::object &JsonValue::object_value() const  {throw std::runtime_error("not a object!");}

const Json &JsonValue::operator[](size_t i) const{
    throw std::runtime_error("not an array");
}

const Json &JsonValue::operator[](const std::string key) const{
    throw std::runtime_error("not an object");
}

template<Json::JsonType tag, typename T>
class Value: public JsonValue{
protected:
    Value(const T& value):m_value(value){}
    Value(T &&value):m_value(std::move(value)){}

    Json::JsonType type() const{
        return tag;
    }

    //reinterpret_cast only guarantees that if you cast a pointer to a different type, and then reinterpret_cast it back to the original type
    bool equals(const JsonValue * other) const {
        return m_value == reinterpret_cast<const Value<tag, T> *>(other)->m_value;
    }
    bool less(const JsonValue * other) const {
        return m_value < reinterpret_cast<const Value<tag, T> *>(other)->m_value;
    }

    virtual void serialize(std::string& out) const{
        lxjson::serialize(m_value, out);
    }


    const T m_value;
};

//Type: number
class JsonInt final : public Value<Json::JsonType::T_NUMBER, int>{
public:
    int int_value() const {return m_value;}//cast
    double double_value() const {return m_value;}
    bool equals(const JsonValue * other) const { return m_value == other->double_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->double_value(); }

    JsonInt(double value):Value(value){}
};

class JsonDouble final : public Value<Json::JsonType::T_NUMBER, double>{
public:
    int int_value() const {return m_value;}//cast
    double double_value() const {return m_value;}
    bool equals(const JsonValue * other) const { return m_value == other->double_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->double_value(); }

    JsonDouble(double value):Value(value){}
};

//Type: bool
class JsonBool final : public Value<Json::JsonType::T_BOOL, bool>{
public:
    bool bool_value() const {return m_value;}

    JsonBool(bool value): Value(value){}
};

//Type string
class JsonString final : public Value<Json::JsonType::T_STRING, string>{
public:
    const string &string_value() const {return m_value;}

    JsonString(const string &value): Value(value){}
    JsonString(string &&value): Value(std::move(value)){}
};

class JsonArray final : public Value<Json::JsonType::T_ARRAY, Json::array> {
public:
    const Json::array &array_value() const {return m_value;}
    const Json &operator[](size_t i) const {
        if (i >= m_value.size()) return json_null;
        return m_value[i];
    }

    JsonArray(const Json::array &value): Value(value){}
    JsonArray(Json::array &&value): Value(std::move(value)){}
};

class JsonObject final : public Value<Json::JsonType::T_OBJECT, Json::object> {
public:
    const Json::object &object_value() const{
        return m_value;
    }
    //std::map's operator [] is not declared as const, and cannot be due to its behavior
    //in C++11, you can use the at() operator
    const Json& operator[](const string &key) const{
        return m_value.at(key);
    }

    JsonObject(const Json::object &value): Value(value){}
    JsonObject(Json::object &&value) : Value(value){}
};

class JsonNull final : public Value<Json::JsonType::T_NULL, std::nullptr_t> {
public:
    JsonNull() : Value(nullptr) {}
};

//Json constructor
static const std::shared_ptr<JsonValue> obj_null(std::make_shared<JsonNull>());
static const std::shared_ptr<JsonValue> obj_true(std::make_shared<JsonBool>(true));
static const std::shared_ptr<JsonValue> obj_false(std::make_shared<JsonBool>(false));

Json::Json() noexcept               : jv_ptr(obj_null){}
Json::Json(std::nullptr_t) noexcept : jv_ptr(obj_null){}
Json::Json(double value)            : jv_ptr(std::make_shared<JsonDouble>(value)){}
Json::Json(int value)               : jv_ptr(std::make_shared<JsonInt>(value)){} 
Json::Json(bool value)              : jv_ptr(value ? obj_true : obj_false){}
Json::Json(const std::string& value): jv_ptr(std::make_shared<JsonString>(value)){}
Json::Json(std::string&& value)     : jv_ptr(std::make_shared<JsonString>(std::move(value))) {}
Json::Json(const char* value)       : jv_ptr(std::make_shared<JsonString>(value)){}
Json::Json(const array& value)      : jv_ptr(std::make_shared<JsonArray>(value)){}
Json::Json(array&& value)           : jv_ptr(std::make_shared<JsonArray>(std::move(value))){}
Json::Json(const object & value)    : jv_ptr(std::make_shared<JsonObject>(value)){}
Json::Json(object && value)         : jv_ptr(std::make_shared<JsonObject>(std::move(value))){}


Json::Json(const Json& rhs){
    switch(rhs.type()){
        case JsonType::T_NULL:
            jv_ptr = obj_null;
            break;
        case JsonType::T_NUMBER:
            jv_ptr = std::make_shared<JsonDouble>(rhs.double_value());
            break;
        case JsonType::T_BOOL:
            jv_ptr = (rhs.bool_value()? obj_true : obj_false);
            break;
        case JsonType::T_STRING:
            jv_ptr = std::make_shared<JsonString>(rhs.string_value());
            break;
        case JsonType::T_ARRAY:
            jv_ptr = std::make_shared<JsonArray>(rhs.array_value());
            break;
        case JsonType::T_OBJECT:
            jv_ptr = std::make_shared<JsonObject>(rhs.object_value());
            break;        
    }
}

Json::Json(Json&& rhs) noexcept : jv_ptr(std::move(rhs.jv_ptr)) {}
Json::~Json() {}

Json& Json::operator=(Json rhs){
    using std::swap;
    swap(jv_ptr, rhs.jv_ptr);
}

//comparison
bool Json::operator==(const Json& rhs) const{
    if (jv_ptr->type() != rhs.type())
        return false;
    return jv_ptr->equals(rhs.jv_ptr.get());
}

bool Json::operator>(const Json& rhs) const{
    if (jv_ptr->type() != rhs.type())
        return false;
    return rhs.jv_ptr->less(jv_ptr.get());
}

//check type
Json::JsonType Json::type()   const {return jv_ptr->type();}
bool Json::is_null()    const {return type()==JsonType::T_NULL;}
bool Json::is_number()  const {return type()==JsonType::T_NUMBER;}
bool Json::is_bool()    const {return type()==JsonType::T_BOOL;}
bool Json::is_string()  const {return type()==JsonType::T_STRING;}
bool Json::is_array()   const {return type()==JsonType::T_ARRAY;}
bool Json::is_object()  const {return type()==JsonType::T_OBJECT;}

//get value
int Json::int_value()                       const {return jv_ptr->int_value();}
double Json::double_value()                 const {return jv_ptr->double_value();}
const string &Json::string_value()          const {return jv_ptr->string_value();}
bool Json::bool_value()                     const {return jv_ptr->bool_value();}
const Json::array &Json::array_value()      const {return jv_ptr->array_value();}
const Json::object &Json::object_value()    const {return jv_ptr->object_value();}

const Json& Json::operator[](size_t i) const{
    return jv_ptr->operator[](i);

}
const Json& Json::operator[](const std::string key) const{
    return jv_ptr->operator[](key);
}

void Json::serialize(std::string &out) const{
    jv_ptr->serialize(out);
}


//todo:
class Parser;
/*
Json Json::parse(const string &in, string &err) {
    try {
        int i=0;
        //Parser p(in);
        //return p.parse();
    } catch (std::runtime_error& e) {
        //std::runtime_error& e;
        err = e.what();
        return json_null;
    }

}
*/


}
