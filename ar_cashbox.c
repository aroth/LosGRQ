#include "g_local.h"

void cashbox_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf){
}

void cashbox_think( edict_t *ent ){

}

void coin_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf){
}

void coin_think( edict_t *ent ){

}

void Cmd_CashOut( edict_t *ent ){
	int i=0;
	int count = 5; //ent->client->coins_in_hand;
	vec3_t a;
	
	if( count == 0){
		gi.centerprintf(ent, "You have 0 coins.");
		return;
	}

	VectorCopy( ent->client->v_angle, a );

	for( i=0; i<count; i++ ){
		gitem_t *it = FindItem("Coin $1");
		edict_t *coin = G_Spawn();
		vec3_t forward, right, offset;

		coin->classname = it->classname;
		coin->item = it;
		coin->mass = 10.0;
		coin->spawnflags = DROPPED_ITEM;
		coin->health = 99999;
		coin->s.effects = it->world_model_flags;
		coin->s.renderfx = RF_GLOW;
		VectorSet( coin->mins, -15, -15, -15 );
		VectorSet( coin->maxs, 15, 15, 15 );
		gi.setmodel( coin, coin->item->world_model);
		coin->solid = SOLID_TRIGGER; // SOLID_TRIGGER;
		coin->takedamage = DAMAGE_YES;
		coin->movetype = MOVETYPE_FLYRICOCHET;
		coin->touch = coin_touch; //drop_temp_touch;
		coin->owner = ent;

		a[1] += (360 / count);

		if (ent->client)
		{
			trace_t	trace;

			AngleVectors (a, forward, right, NULL);
			VectorSet(offset, 24, 0, -16);
			G_ProjectSource (ent->s.origin, offset, forward, right, coin->s.origin);
			trace = gi.trace (ent->s.origin, coin->mins, coin->maxs, coin->s.origin, ent, CONTENTS_SOLID);
			VectorCopy (trace.endpos, coin->s.origin);

			coin->s.origin[2] += 300;
//			VectorScale( forward, 20 + (i*5), coin->velocity );
		}

		coin->nextthink = level.time + 3; // wait 3 sec?
		coin->think =  coin_think; //coin_touch; //sdrop_make_touchable;

		gi.linkentity (coin);
	//ent->client->coins_in_hand -= 1;

	}
//	return dropped;
}
