#include "game/gamestate.h"
#include "game/city/city.h"
#include "game/city/vequipment.h"
#include "game/city/building.h"
#include "game/city/vehicle.h"
#include "game/organisation.h"
#include "game/base/base.h"
#include "game/rules/vequipment.h"

#include "framework/includes.h"
#include "framework/framework.h"

#include <random>

namespace OpenApoc
{

GameState::GameState(Framework &fw, Rules &rules)
    : player(nullptr), showTileOrigin(false), showVehiclePath(false), showSelectableBounds(false),
      rng(std::random_device{}())
{
	for (auto &org : rules.getOrganisations())
	{
		if (this->organisations.find(org.ID) != this->organisations.end())
		{
			LogError("Multiple organisations with ID \"%s\"", org.ID.c_str());
		}
		this->organisations[org.ID] = std::make_shared<Organisation>(org);
		/* FIXME: Make 'player' organisation selectable? */
		if (org.ID == "ORG_X-COM")
		{
			this->player = this->organisations[org.ID];
		}
	}

	if (!this->player)
	{
		LogError("No player organisation defined");
	}

	this->city.reset(new City(fw, *this));

	// Place some random testing vehicles
	std::uniform_int_distribution<int> bld_distribution(0, this->city->buildings.size() - 1);

	// Loop through all vehicle types and weapons to get a decent spread for testing
	auto vehicleTypeIt = rules.getVehicleTypes().begin();

	for (int i = 0; i < 100; i++)
	{
		while (vehicleTypeIt->second->type != VehicleType::Type::Flying)
		{
			vehicleTypeIt++;
			if (vehicleTypeIt == rules.getVehicleTypes().end())
				vehicleTypeIt = rules.getVehicleTypes().begin();
		}
		auto owner = this->getOrganisation(vehicleTypeIt->second->manufacturer);
		// We add player vehicles a bit later
		if (owner == this->getPlayer())
		{
			vehicleTypeIt++;
			continue;
		}

		auto testVehicle = std::make_shared<Vehicle>(*vehicleTypeIt->second, owner);

		testVehicle->equipDefaultEquipment(rules);

		this->city->vehicles.push_back(testVehicle);
		owner->vehicles.push_back(testVehicle);
		auto b = this->city->buildings[bld_distribution(rng)];
		b->landed_vehicles.insert(testVehicle);
		testVehicle->building = b;

		// Initialise these vehicles with a random weapon cooldown to stop them all firing at
		// exactly the same time when starting the game
		for (auto e : testVehicle->equipment)
		{

			if (e->type.type != VEquipmentType::Type::Weapon)
			{
				continue;
			}
			auto weapon = std::dynamic_pointer_cast<VWeapon>(e);

			auto &weaponType = static_cast<const VWeaponType &>(weapon->type);

			std::uniform_int_distribution<int> reload_distribution(0, weaponType.fire_delay *
			                                                              TICK_SCALE);
			int initialDelay = reload_distribution(rng);
			if (initialDelay != 0)
			{
				weapon->setReloadTime(initialDelay);
			}
		}

		vehicleTypeIt++;
	}

	if (this->city->baseBuildings.empty())
	{
		LogError("No valid base buildings");
	}
	std::uniform_int_distribution<int> base_distribution(0, this->city->baseBuildings.size() - 1);
	auto base = this->city->baseBuildings[base_distribution(rng)]->base;
	this->playerBases.emplace_back(base);
	base->name = "Test Base";
	base->bld.lock()->owner = this->getPlayer();
	base->startingBase(fw, rng);

	// Give the player some extra vehicles to test the equipment stuff (as the x-com built vehicles
	// don't come with much)

	for (auto &it : rules.getVehicleTypes())
	{
		auto &vType = *it.second;
		// Only give stuff that can be equipped
		if (!vType.equipment_screen)
			continue;
		auto owner = this->getPlayer();
		auto testVehicle = std::make_shared<Vehicle>(vType, owner);
		testVehicle->equipDefaultEquipment(rules);
		this->city->vehicles.push_back(testVehicle);
		owner->vehicles.push_back(testVehicle);
		auto b = base->bld.lock();
		b->landed_vehicles.insert(testVehicle);
		testVehicle->building = b;
	}

	// Give that base some inventory
	for (auto &pair : rules.getVehicleEquipmentTypes())
	{
		auto &equipmentID = pair.first;
		base->inventory[equipmentID] = 10;
	}
}

// Just a handy shortcut since it's shown on every single screen
UString GameState::getPlayerBalance() const
{
	return Strings::FromInteger(this->getPlayer()->balance);
}

sp<Organisation> GameState::getOrganisation(const UString &orgID)
{
	auto f = this->organisations.find(orgID);
	if (f == this->organisations.end())
	{
		LogError("No organisation matching ID \"%s\"", orgID.c_str());
	}
	return f->second;
}

sp<Organisation> GameState::getPlayer() const { return this->player; }

}; // namespace OpenApoc
