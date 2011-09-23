#include "g_local.h"

void cashbox_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf){
	if( other == ent->owner ){
		return;
	}
	if (!other->takedamage)
	{
		VectorClear(ent->velocity);
		VectorClear(ent->avelocity);
		ent->movetype = MOVETYPE_NONE;
	}
	return;
}

void cashbox_think( edict_t *ent ){
	ent->nextthink = level.time + 0.1;
}


void coin_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf){
	if( other->client ){
		gi.sound(ent, CHAN_ITEM, gi.soundindex("coin/chching1.wav"), 1, ATTN_NORM, 0);
		other->client->cash_in_hand += 1;
		
		// aroth: this will be too frequent if there are a lot of coins. ok for now.
		if( ent->owner == other ){
			gi.bprintf(PRINT_MEDIUM, "%s recovered his own cash.\n", other->client->pers.netname);
		}else{
			ent->owner = other; // change ownership
			ent->client->resp.cash_stolen +=1; // track steals
			gi.bprintf(PRINT_MEDIUM, "%s accepted %s's spare change.\n", other->client->pers.netname, ent->owner->client->pers.netname);
		}

		gi.unlinkentity( ent );
		G_FreeEdict( ent );
	}
}

void coin_think( edict_t *ent ){
}


void Cmd_Cashbox( edict_t *ent ){
	gclient_t *client = ent->client;
	gi.centerprintf( ent, "HAS CASHBOX? %d", client->resp.has_cashbox);
	if( client->resp.has_cashbox == true ){
		
		vec3_t forward, right, offset;	
		trace_t tr;

		edict_t *cashbox = G_Spawn();

		cashbox->owner = ent;
		cashbox->spawnflags = DROPPED_ITEM;
		cashbox->classname = "cashbox";
		cashbox->s.modelindex = gi.modelindex("models/items/box_red/tris.md2");
		cashbox->s.skinnum = 0;
		cashbox->movetype = MOVETYPE_TOSS;
		cashbox->clipmask = MASK_SOLID;
		cashbox->solid = SOLID_BBOX;

		VectorSet( cashbox->mins, -15, -15, -15 );
		VectorSet( cashbox->maxs, 15, 15, 15 );

		AngleVectors( client->v_angle, forward, right, NULL );
		VectorSet( offset, 0, 0, 0 );
		G_ProjectSource( ent->s.origin, offset, forward, right, cashbox->s.origin );
		VectorScale( forward, 200, cashbox->velocity );
		cashbox->velocity[2] = 100;

		cashbox->think = cashbox_think;
		cashbox->nextthink = level.time + 0.1;
		cashbox->touch = cashbox_touch;

		gi.linkentity( cashbox );

		client->resp.cash_in_box += client->cash_in_hand;
		client->cash_in_hand = 0;
		
		gi.centerprintf(ent, "Dropped cashbox...");
		client->resp.has_cashbox = false;
	}else{
		
		float radius = 64.0;
		edict_t *e = NULL;
		while ((e = findradius(e, ent->s.origin, radius)) != NULL){
			if( strcmp( e->classname, "cashbox") == 0 && e->owner == ent ){
				gi.unlinkentity( e );
				G_FreeEdict( e );
				gi.centerprintf(ent, "Picked up cashbox...");
				client->resp.has_cashbox = true;
				client->cash_in_hand += client->resp.cash_in_box;
				client->resp.cash_in_box = 0;
				break;
			}
		}
	}
}

void Cmd_GiveCash( edict_t *ent ){
	if( ent->client ){
		ent->client->cash_in_hand += 13;
		makeChange( ent );
	}
}

void Cmd_CashOut( edict_t *ent ){

	vec3_t a;
	int i=0;
	int count = ent->client->cash_in_hand;

	VectorCopy( ent->client->v_angle, a );
	
	makeChange( ent );

	for( i=0; i<count; i++ ){
		gitem_t *it = FindItem("Coin $1");
		edict_t *coin = G_Spawn();
		vec3_t forward, right, offset;

		coin->classname = it->classname;
		coin->item = it;
		coin->mass = 50.0;
		coin->spawnflags = DROPPED_ITEM;
		coin->health = 99999;
		coin->s.effects = it->world_model_flags;
		coin->s.renderfx = RF_GLOW;
		VectorSet( coin->mins, -15, -15, -15 );
		VectorSet( coin->maxs, 15, 15, 15 );
		gi.setmodel( coin, coin->item->world_model);
		coin->solid = SOLID_TRIGGER;
		coin->takedamage = DAMAGE_YES;
		coin->movetype = MOVETYPE_FLYRICOCHET;
		coin->touch = coin_touch;
		coin->owner = ent;

		a[1] += (360 / count);

		AngleVectors(a, forward, right, NULL);
		VectorSet( offset, 0, 48, 48 );
		G_ProjectSource( ent->s.origin, offset, forward, right, coin->s.origin);

		gi.linkentity (coin);
	}
}

void makeChange( edict_t *ent ){
	int cash = ent->client->cash_in_hand;

	int c20 = 0;
	int c10 = 0;
	int c5  = 0;
	int c1  = 0;

	c20 = floor( cash / 20 );
	cash -= (c20 * 20);

	c10 = floor( cash / 10 );
	cash -= (c10 * 10);

	c5  = floor( cash / 5 );
	cash -= (c5 * 5);

	c1  = floor( cash / 1 );
	cash -= (c1 * 1);
	
	ent->client->cash_20s = c20;
	ent->client->cash_10s = c10;
	ent->client->cash_5s = c5;
	ent->client->cash_1s = c1;

	gi.dprintf("%d 20s, %d 10s, %d 5s, %d 1s\n", c20, c10, c5, c1);
}
