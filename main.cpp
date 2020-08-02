
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
#include <csignal>
#include "libspi-bme680.h"

namespace fs = std::filesystem;
using system_clock = std::chrono::system_clock;
using time_point = std::chrono::system_clock::time_point;
using data_point = std::pair<time_point, Measurement>;

// Globals:
std::vector<data_point> measurements;
std::ofstream out;

// Forward Declarations:
extern "C" {
void sig_handler(int s);
}

void flush_data();


// Definitions:
void sig_handler(int s) {
	std::cerr << "Caught signal " << s << std::endl;
	flush_data();
	if (s == SIGUSR1 || s == SIGUSR2) {
		return;	// flush, but keep service alive on user signals.
	}
	std::exit(EXIT_SUCCESS);	// indicate successful flush
}


void flush_data() {
	for (const auto [t, m] : measurements) {
		const std::time_t t_c = system_clock::to_time_t(t);
		out << std::put_time(std::localtime(&t_c), "%F %T,") << m << '\n';
	}
	measurements.clear();
	out.flush();
}


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
	out.open(dataFile, std::ios_base::app);
	if (setupDataFile) {
		out << "Time,Temperature (C),Pressure (hPa),Relative Humidity (%),Gas Sensor (ohms)\n";
	}

	// Register SIGINT handler for clean shutdown:
	struct sigaction sigHandler;
	sigHandler.sa_handler = sig_handler;
	sigemptyset(&sigHandler.sa_mask);
	sigHandler.sa_flags = 0;
	sigaction(SIGINT, &sigHandler, NULL);
	sigaction(SIGTERM, &sigHandler, NULL);
	sigaction(SIGQUIT, &sigHandler, NULL);
	sigaction(SIGHUP, &sigHandler, NULL);
	sigaction(SIGUSR1, &sigHandler, NULL);
	sigaction(SIGUSR2, &sigHandler, NULL);

	// Sensor measurement loop:
	measurements.reserve(60);
	time_point next = std::chrono::floor<std::chrono::minutes>(start) + 1min;
	for (;;) {
		std::this_thread::sleep_until(next);

		// Perform measurement:
		measurements.emplace_back(next, sensor.measure());
		next += 1min;

		// Flush measurements to file:
		if (measurements.size() >= 60) {
			flush_data();
		}
	}
}

