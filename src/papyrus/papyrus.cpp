#include "papyrus/papyrus.h"
#include "papyrus/plugin.h"
#include "papyrus/scale.h"
#include "papyrus/height.h"
#include "papyrus/events.h"
#include "papyrus/camera.h"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {

}

namespace Gts {
	bool register_papyrus(IVirtualMachine* vm) {
		register_papyrus_plugin(vm);
		register_papyrus_scale(vm);
		register_papyrus_height(vm);
		register_papyrus_events(vm);
		register_papyrus_camera(vm);
		return true;
	}
}
