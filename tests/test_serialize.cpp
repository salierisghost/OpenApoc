#include "framework/configfile.h"
#include "framework/filesystem.h"
#include "framework/framework.h"
#include "framework/logger.h"
#include "game/state/gamestate.h"
#include "game/state/gamestate_serialize.h"
#include <iostream>
#include <sstream>

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>

// We can't just use 'using namespace OpenApoc;' as:
// On windows VS it says
/* Error	C2678	binary '==': no operator found which takes a left - hand operand of type
'Concurrency::details::_Task_impl<_ReturnType>'
(or there is no acceptable conversion)	test_serialize	E :
\Projects\GitHub\OpenApoc\game\state\gamestate_serialize.h	106

My (JonnyH) assumption is that some part of the system library is defining some operator== for
std::shared_ptr<SomeInternalConcurrencyType>, but it resolves to our OpenApoc operator== instead.
*/

bool test_gamestate_serialization_roundtrip(OpenApoc::sp<OpenApoc::GameState> state,
                                            OpenApoc::UString save_name)
{
	if (!state->saveGame(save_name))
	{

		LogWarning("Failed to save packed gamestate");
		return false;
	}

	auto read_gamestate = OpenApoc::mksp<OpenApoc::GameState>();
	if (!read_gamestate->loadGame(save_name))
	{
		LogWarning("Failed to load packed gamestate");
		return false;
	}

#if 0
	// FIXME: This isn't reliable due to undefined order of containers
	if (*state != *read_gamestate)
#endif
	if (0)
	{
		LogWarning("Gamestate changed over serialization");

		return false;
	}
	return true;
}

bool test_gamestate_serialization(OpenApoc::sp<OpenApoc::GameState> state)
{

	std::stringstream ss;
	ss << "openapoc_test_serialize-" << std::this_thread::get_id();
	auto tempPath = fs::temp_directory_path() / ss.str();
	OpenApoc::UString pathString(tempPath.string());
	LogInfo("Writing temp state to \"%s\"", pathString);
	if (!test_gamestate_serialization_roundtrip(state, pathString))
	{
		LogWarning("Packed save test failed");
		return false;
	}

	fs::remove(tempPath);

	return true;
}

BOOST_TEST_DONT_PRINT_LOG_VALUE(OpenApoc::GameState)

BOOST_AUTO_TEST_CASE(testLoadState)
{
	auto gamestate_name = OpenApoc::config().getString("gamestate");
	auto common_name = OpenApoc::config().getString("common");

	LogInfo("Loading \"%s\"", gamestate_name);

	auto state = OpenApoc::mksp<OpenApoc::GameState>();

	{
		auto state2 = OpenApoc::mksp<OpenApoc::GameState>();
		BOOST_TEST(*state == *state2, "Empty gamestate failed comparison");
	}
	BOOST_TEST(state->loadGame(common_name), "Failed to load gamestate_common");
	BOOST_TEST(state->loadGame(gamestate_name), "Failed to load supplied gamestate");
}

BOOST_AUTO_TEST_CASE(testNoStartNoInitState)
{
	auto state = OpenApoc::mksp<OpenApoc::GameState>();
	LogInfo("Testing non-started non-inited state");
	BOOST_TEST(test_gamestate_serialization(state),
	           "Serialization test failed for non-started non-inited game");
}

BOOST_AUTO_TEST_CASE(testStartedNoInitState)
{
	auto state = OpenApoc::mksp<OpenApoc::GameState>();
	LogInfo("Testing started non-inited state");
	state->startGame();
	BOOST_TEST(test_gamestate_serialization(state),
	           "Serialization test failed for started non-inited game");
}

BOOST_AUTO_TEST_CASE(testStartedInitedState)
{
	auto state = OpenApoc::mksp<OpenApoc::GameState>();
	LogInfo("Testing started inited state");
	state->initState();
	state->fillOrgStartingProperty();
	state->fillPlayerStartingProperty();
	BOOST_TEST(test_gamestate_serialization(state),
	           "Serialization test failed for started inited game");
}

BOOST_AUTO_TEST_CASE(testBattleState)
{
	auto state = OpenApoc::mksp<OpenApoc::GameState>();
	LogInfo("Testing state with battle");
	LogInfo("--Test disabled until we find a way to compare sets properly (fails in sets of "
	        "pointers like hazards)--");
	if (false)
	{

		OpenApoc::StateRef<OpenApoc::Organisation> org = {state.get(),
		                                                  OpenApoc::UString("ORG_ALIEN")};
		auto v = OpenApoc::mksp<OpenApoc::Vehicle>();
		auto vID = OpenApoc::Vehicle::generateObjectID(*state);
		OpenApoc::sp<OpenApoc::VehicleType> vType;

		// Fine a vehicle type with a battlemap
		for (auto &vTypePair : state->vehicle_types)
		{
			if (vTypePair.second->battle_map)
			{
				vType = vTypePair.second;
				break;
			}
		}
		BOOST_TEST(vType, "No vehicle with BattleMap found");
		LogInfo("Using vehicle map for \"%s\"", vType->name);
		v->type = {state.get(), vType};
		v->name = format("%s %d", v->type->name, ++v->type->numCreated);
		state->vehicles[vID] = v;

		OpenApoc::StateRef<OpenApoc::Vehicle> enemyVehicle = {state.get(), vID};
		OpenApoc::StateRef<OpenApoc::Vehicle> playerVehicle = {};

		std::list<OpenApoc::StateRef<OpenApoc::Agent>> agents;
		for (auto &a : state->agents)
		{

			if (a.second->type->role == OpenApoc::AgentType::Role::Soldier &&
			    a.second->owner == state->getPlayer())
			{
				agents.emplace_back(state.get(), a.second);
			}
		}

		OpenApoc::Battle::beginBattle(*state, false, org, agents, nullptr, playerVehicle,
		                              enemyVehicle);
		OpenApoc::Battle::enterBattle(*state);

		BOOST_TEST(test_gamestate_serialization(state),
		           "Serialization test failed for in-battle game");
		OpenApoc::Battle::finishBattle(*state);
		OpenApoc::Battle::exitBattle(*state);
	}
}

OpenApoc::sp<OpenApoc::Framework> f = nullptr;
bool init_unit_test()
{
	OpenApoc::config().addPositionalArgument("common", "Common gamestate to load");
	OpenApoc::config().addPositionalArgument("gamestate", "Gamestate to load");

	auto &suite = boost::unit_test::framework::master_test_suite();

	if (OpenApoc::config().parseOptions(suite.argc, suite.argv))
	{
		return false;
	}

	auto gamestate_name = OpenApoc::config().getString("gamestate");
	if (gamestate_name.empty())
	{
		std::cerr << "Must provide gamestate" << std::endl;
		OpenApoc::config().showHelp();
		return false;
	}
	auto common_name = OpenApoc::config().getString("common");
	if (common_name.empty())
	{
		std::cerr << "Must provide common gamestate\n" << std::endl;
		OpenApoc::config().showHelp();
		return false;
	}

	f = OpenApoc::mksp<OpenApoc::Framework>("OpenApoc", false);

	return true;
}
