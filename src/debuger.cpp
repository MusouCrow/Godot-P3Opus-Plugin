#include "debuger.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Debuger::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add"), &Debuger::add);
    ClassDB::bind_method(D_METHOD("get"), &Debuger::get);
}

Debuger::Debuger() {
    _value = 0;
}

Debuger::~Debuger() {

}

void Debuger::add() {
    _value++;
}

int Debuger::get() {
    return _value;
}
