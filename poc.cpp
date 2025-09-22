#pragma leco app
#pragma leco add_plugin "poc-555-smd.cpp"
#pragma leco add_plugin "poc-555-tht.cpp"
#pragma leco add_plugin "poc-counter.cpp"
#pragma leco add_plugin "poc-skeet.cpp"
#pragma leco add_plugin "poc-tp4056.cpp"
#pragma leco add_plugin "poc-ucamco.cpp"
import casein;
import gerbyv;

static gerby::thread t{"poc-counter"};
