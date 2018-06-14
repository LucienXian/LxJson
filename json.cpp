#include "json.hpp"

namespace lxjson{

using std::string;
using std::vector;
using std::map;

static void serialize(std::nullptr_t, string &out){}
static void serialize(int values, string &out){}
static void serialize(double values, string &out){}
static void serialize(bool values, string &out){}
static void serialize(const string &values, string &out){}
static void serialize(const Json::array &values, string &out){}
static void serialize(const Json::object &values, string &out){}

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
    int int_value() const {return m_value;}//cast
    double double_value() const {return m_value;}
    bool equals(const JsonValue * other) const { return m_value == other->double_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->double_value(); }

    JsonInt(double value):Value(value){}
};

class JsonDouble final : public Value<Json::JsonType::T_NUMBER, double>{
    int int_value() const {return m_value;}//cast
    double double_value() const {return m_value;}
    bool equals(const JsonValue * other) const { return m_value == other->double_value(); }
    bool less(const JsonValue * other)   const { return m_value <  other->double_value(); }

    JsonDouble(double value):Value(value){}
};

//Type: bool
class JsonBool final : public Value<Json::JsonType::T_BOOL, bool>{
    bool bool_value() const {return m_value;}

    JsonBool(bool value): Value(value){}
};

//Type string
class JsonString final : public Value<Json::JsonType::T_STRING, string>{
    const string &string_value() const {return m_value;}

    JsonString(const string &value): Value(value){}
    JsonString(string &&value): Value(std::move(value)){}
};

class JsonArray final : public Value<Json::JsonType::T_ARRAY, Json::array> {
    const Json::array &array_value() const {return m_value;}
    const Json &operator[](size_t i) const {return m_value[i];}

    JsonArray(const Json::array &value): Value(value){}
    JsonArray(Json::array &&value): Value(std::move(value)){}
};

class JsonObject final : public Value<Json::JsonType::T_OBJECT, Json::object> {
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

}
