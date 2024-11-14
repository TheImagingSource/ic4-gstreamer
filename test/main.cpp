
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <chrono>
#include <iostream>

#include <gst/gst.h>

int main(int argc, char** argv)
{

	gst_init(&argc, &argv);

	using namespace std::chrono;
	auto t0 = high_resolution_clock::now();
    
    int ret = doctest::Context(argc, argv).run();
    
	auto t1 = high_resolution_clock::now();
	auto dt_ms = duration_cast<milliseconds>(t1 - t0).count();

    std::cout << "Total time" << static_cast<double>(dt_ms / 1e3) << " s" << std::endl;
    
    return ret;
}
