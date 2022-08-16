#include "Config.hpp"
// Config file that is loaded a init
#include <articuno/archives/ryml/ryml.h>

using namespace articuno::ryml;
using namespace Gts;

const Config& Config::GetSingleton() noexcept {
	static Config instance;

	static std::atomic_bool initialized;
	static std::latch latch(1);
	if (!initialized.exchange(true)) {
		std::ifstream inputFile(R"(Data\SKSE\Plugins\GtsPlugin.yaml)");
		if (inputFile.good()) {
			yaml_source Ar(inputFile);
			ar >> instance;
		}
		latch.count_down();
	}
	latch.wait();

	return instance;
}
