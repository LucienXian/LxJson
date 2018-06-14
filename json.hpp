#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace lxjson{

class JsonValue;

class Json final {
public:
    /*c++11 enum class: types*/
    enum class JsonType {
        T_NULL = 1,
        T_NUMBER = 2,
        T_BOOL = 4,
        T_STRING = 8,
        T_ARRAY = 16,
        T_OBJECT = 32
    };


    //Prefer alias declarations to typedefs
    using array = std::vector<Json>;
    using object = std::map<std::string, Json>;

    //constructors
    explicit Json() noexcept;
    explicit Json(std::nullptr_t) noexcept;//nullptr is a C++ keyword literal of type std::nullptr_t
    explicit Json(double);
    explicit Json(int);
    explicit Json(bool);
    explicit Json(const std::string&);
    explicit Json(std::string&&);
    explicit Json(const char*);
    explicit Json(const array&);
    explicit Json(array&&);
    explicit Json(const object &);
    explicit Json(object &&);

    //copy constructor
    Json(const Json&);
    //move constructor
    Json(Json&&) noexcept;

    ~Json();

    Json& operator=(Json);

    bool operator==(const Json&) const;
    bool operator>(const Json&) const;
    bool operator!=(const Json& rhs) const {return !(rhs==*this);}
    bool operator>=(const Json& rhs) const {return !(rhs>*this);}
    bool operator<(const Json& rhs) const {return rhs>*this;}
    bool operator<=(const Json& rhs) const {return !(*this>rhs);}

    //Type
    JsonType type() const;
    bool is_null() const;
    bool is_number() const; 
    bool is_bool() const;
    bool is_string() const;
    bool is_array() const;
    bool is_object() const;

    //number-type
    int int_value() const;
    double double_value() const;
    //bool-value
    bool bool_value() const;
    //string-type
    const std::string& string_value() const;
    //array-type
    const array& array_value() const;
    //object-type
    const object& object_value() const;

    //return arr[i] if this is an array
    const Json& operator[](size_t i) const;
    //return obj[key] if this is an object
    const Json& operator[](const std::string key) const;

    //Serialize
    void serialize(std::string &out) const;
    std::string serialize() const{
        std::string out;
        serialize(out);
        return out;
    }

    //parse the c++ string, if error happens, storage the message in the err
    static Json parse(const std::string &in, std::string& err);
    static Json parse(const char* in, std::string& err){
        if (in)
            return parse(std::string(in), err);
        else{
            err = "null input";
            return Json(nullptr);
        }
    }

private:
    std::shared_ptr<JsonValue> jv_ptr;

};

class JsonValue{
protected:
    friend class Json;
    friend class JsonInt;
    friend class JsonDouble;
    virtual Json::JsonType type() const = 0;
    virtual bool equals(const JsonValue *other) const = 0;
    virtual bool less(const JsonValue *other) const = 0;
    virtual void serialize(std::string& out) const = 0;

    virtual double double_value() const;
    virtual int int_value() const;
    virtual bool bool_value() const;
    virtual const std::string &string_value() const;
    virtual const Json::array &array_value() const;
    virtual const Json::object &object_value() const;

    virtual const Json &operator[](size_t i) const;
    virtual const Json &operator[](const std::string key) const;

    virtual ~JsonValue(){}
    JsonValue(){}
};

}