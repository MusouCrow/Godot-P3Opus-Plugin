#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class Debuger : public RefCounted {
    GDCLASS(Debuger, RefCounted);

protected:
    static void _bind_methods();

private:
    int _value;

public:
    Debuger();
    ~Debuger();

    void add();
    int get();
};
