#include "stats_recorder.hpp"

#include "common.hpp"

void StatsRecorder::damagePotential(Worm* byWorm, WormWeapon* weapon, int hp)
{
}

void StatsRecorder::damageDealt(Worm* byWorm, WormWeapon* weapon, Worm* toWorm, int hp, bool hasHit)
{
}
	
void StatsRecorder::shot(Worm* byWorm, WormWeapon* weapon)
{
}

void StatsRecorder::hit(Worm* byWorm, WormWeapon* weapon, Worm* toWorm)
{
}

void StatsRecorder::afterSpawn(Worm* worm)
{
}

void StatsRecorder::afterDeath(Worm* worm)
{
}

void StatsRecorder::finish()
{

}

void StatsRecorder::tick()
{

}

void StatsRecorder::write(Common& common, gvl::stream_ptr sink)
{
}

void NormalStatsRecorder::damagePotential(Worm* byWorm, WormWeapon* weapon, int hp)
{
	if (!byWorm || !weapon)
		return;

	WormStats& ws = worms[byWorm->index];
	WeaponStats& weap = ws.weapons[weapon->id];
	weap.potentialHp += hp;
}

void NormalStatsRecorder::damageDealt(Worm* byWorm, WormWeapon* weapon, Worm* toWorm, int hp, bool hasHit)
{
	assert(toWorm);

	worms[toWorm->index].damage += hp;
	if(byWorm)
	{
		if(byWorm != toWorm)
			worms[byWorm->index].damageDealt += hp;
		else
			worms[byWorm->index].selfDamage += hp;
	}

	if(!byWorm || !weapon)
		return;

	if(byWorm != toWorm) // Don't count if projectile already hit
	{
		WormStats& ws = worms[byWorm->index];
		WeaponStats& weap = ws.weapons[weapon->id];
		if (!hasHit)
			weap.actualHp += hp;
		weap.totalHp += hp;
	}
}
	
void NormalStatsRecorder::shot(Worm* byWorm, WormWeapon* weapon)
{
	if (!byWorm || !weapon)
		return;

	WormStats& ws = worms[byWorm->index];
	WeaponStats& weap = ws.weapons[weapon->id];
	weap.potentialHits += 1;
}

void NormalStatsRecorder::hit(Worm* byWorm, WormWeapon* weapon, Worm* toWorm)
{
	assert(toWorm);

	if (!byWorm || !weapon)
		return;

	if (byWorm != toWorm)
	{
		WormStats& ws = worms[byWorm->index];
		WeaponStats& weap = ws.weapons[weapon->id];
		weap.actualHits += 1;
	}
}

void NormalStatsRecorder::afterSpawn(Worm* worm)
{
	WormStats& w = worms[worm->index];
	w.spawnTime = frame;
}

void NormalStatsRecorder::afterDeath(Worm* worm)
{
	WormStats& w = worms[worm->index];
	w.lifeSpans.push_back(std::make_pair(w.spawnTime, frame));
	w.spawnTime = -1;
}

void NormalStatsRecorder::finish()
{
	for (int i = 0; i < 2; ++i)
	{
		WormStats& w = worms[i];
		if (w.spawnTime >= 0)
		{
			w.lifeSpans.push_back(std::make_pair(w.spawnTime, frame));
			w.spawnTime = -1;
		}
	}
}

void NormalStatsRecorder::tick()
{
	++frame;
}

void NormalStatsRecorder::write(Common& common, gvl::stream_ptr sink)
{
	gvl::octet_stream_writer w(sink);

	w << "Stats\n\n";

	for (int i = 0; i < 2; ++i)
	{
		WormStats& worm = worms[i];
		w << "Worm " << i << '\n';
		w << "Damage received: " << worm.damage << '\n';
		w << "Damage dealt: " << worm.damageDealt << '\n';
		w << "Damage to self: " << worm.selfDamage << '\n';

		int min, max;
		worm.lifeStats(min, max);
		w << "Longest life: " << (max + 69) / 70 << " sec\n";
		w << "Shortest life: " << (min + 69) / 70 << " sec\n";

		for (int j = 0; j < 40; ++j)
		{
			WeaponStats& weapon = worm.weapons[j];
			if (weapon.potentialHits > 0)
			{
				w << "Weapon " << common.weapons[j].name << '\n';
				w << "  " << weapon.actualHits << "/" << weapon.potentialHits << " hits (" << (weapon.actualHits * 100 / weapon.potentialHits) << "%)\n";
				if (weapon.potentialHp > 0)
					w << "  " << weapon.actualHp << "/" << weapon.potentialHp << " hp (" << (weapon.actualHp * 100 / weapon.potentialHp) << "%)\n";
				if (weapon.actualHp != weapon.totalHp)
					w << "  " << weapon.totalHp << " total hp\n";
			}
		}
		w << '\n';
	}
}