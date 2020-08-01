
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>

#include <cstdlib>
#include <ctime>
#include "libspi-bme680.h"

namespace fs = std::filesystem;
using system_clock = std::chrono::system_clock;
using time_point = std::chrono::system_clock::time_point;

int main (int argc, char* argv[]) {
	using namespace std::literals::chrono_literals;
	const time_point start = system_clock::now();

	// Sensor initialization:
	fs::path devFile = "/dev/spidev0.0";
	bme680 sensor(devFile);
	sensor.configure();

	// Choose data file:
	fs::path dataFile = "data.txt";
	if (argc > 1)
		dataFile = argv[1];

	// Append to sensor output file:
	bool setupDataFile = !fs::exists(dataFile);
	std::ofstream out(dataFile, std::ios_base::app);
	if (setupDataFile)
		out << "Time,Temperature (C),Pressure (hPa),Relative Humidity (%),Gas Sensor (ohms)\n";

	// Sensor measurement loop:
	time_point next = std::chrono::floor<std::chrono::minutes>(start) + 1min;
	for (;;) {
		std::this_thread::sleep_until(next);
		next += 1min;
		const Measurement m = sensor.measure();

		const std::time_t t_c = system_clock::to_time_t(system_clock::now());
		out << std::put_time(std::localtime(&t_c), "%F %T,") << m << std::endl;
	}
}

