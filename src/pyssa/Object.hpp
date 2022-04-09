#pragma once

#include <Python.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

namespace PySSA {

class Object {
public:
    Object()
        : m_object(nullptr) { }

    ~Object() {
        if (m_object)
            Py_DECREF(m_object);
    }

    Object(Object const& other)
        : m_object(other.m_object) {
        if (m_object)
            Py_INCREF(m_object);
    }

    Object& operator=(Object const& other) {
        if (this == &other)
            return *this;
        m_object = other.m_object;
        if (m_object)
            Py_INCREF(m_object);
        return *this;
    }

    Object(Object&& other)
        : m_object(std::exchange(other.m_object, nullptr)) { }

    Object& operator=(Object&& other) {
        if (this == &other)
            return *this;
        m_object = std::exchange(other.m_object, nullptr);
        return *this;
    }

    // Create a new Object without sharing a reference (take ownership).
    // Use for all Python object that you created and nothing else!
    static Object take(PyObject* object) {
        Object o;
        o.m_object = object;
        return o;
    }

    // Create a new Object and share a reference.
    static Object share(PyObject* object) {
        Object o;
        o.m_object = object;
        Py_INCREF(o.m_object);
        return o;
    }

    static Object create_tuple(Py_ssize_t size) {
        Object o;
        o.m_object = PyTuple_New(size);
        return o;
    }

    static Object create_none();
    static Object create_string(std::string const&);
    static Object create_int(int);
    static Object create_double(double);

    void set_tuple_item(Py_ssize_t i, Object const& object) {
        PyTuple_SetItem(m_object, i, object.share_object());
    }

    // Get access to a Python object without transferring ownership.
    PyObject* python_object() const { return m_object; }

    bool operator!() const { return !m_object; }

    void print() const {
        PyObject_Print(m_object, stdout, 0);
        puts("");
    }

    // Stop owning the object. After that, it's the user responsibility
    // to manage object lifetime.
    PyObject* leak_object() {
        auto object = m_object;
        m_object = nullptr;
        return object;
    }

    // Return an internal Python object, also sharing a reference. Use
    // when calling Python functions that want to share these values
    PyObject* share_object() const {
        Py_INCREF(m_object);
        return m_object;
    }

    Object get_attribute(Object const& name);
    Object get_attribute(std::string const& name) { return get_attribute(Object::create_string(name)); }
    void set_attribute(Object const& name, Object const& value);
    void set_attribute(std::string const& name, Object const& value) { set_attribute(Object::create_string(name), value); }

    Object call(Object const& args, Object const& kwargs = {}) const;

    int as_int() const;
    std::vector<Object> as_list() const;
    std::string str() const;
    std::string repr() const;

private:
    PyObject* m_object; // Initialized in Object()
};

}
