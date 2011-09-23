#include "g_local.h"


void cashbox_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf){
	
	if( ent->goalentity == other ){
		// TODO: prevent flooding of these messages
		gi.centerprintf(other, "Your cashbox has $%d", other->client->resp.cash_in_box);
	}else{
		gi.centerprintf(other, "You have found %s's cashbox!", ent->goalentity->client->pers.netname);
	}

	//if( other->client ){
//		gi.centerprintf( other, "You have found %s's cashbox.", ent->goalentity->client->pers.netname);		
//	}

	/*
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
*/
	
}


void cashbox_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point){
	if( attacker->client ){
		// Print messages
		gi.bprintf(PRINT_MEDIUM, "%s has raided %s's cashbox!", attacker->client->pers.netname, self->goalentity->client->pers.netname);
		gi.centerprintf(self->goalentity, "%s has raided your cashbox!", attacker->client->pers.netname);
		gi.centerprintf(attacker, "You have raided %s's cashbox!", self->goalentity->client->pers.netname);
		
		attacker->client->cash_in_hand = self->goalentity->client->resp.cash_in_box;  // exchange funds
		self->goalentity->client->resp.has_cashbox = true;							   // return cashbox to victim
		self->goalentity->client->resp.cash_in_box = 0;								   // ...empty

	
	}else{ 
		// lava? slime? 
		gi.dprintf("Cashbox died in another way. Handle this. with %s", attacker->classname); //TODO
	}

	gi.unlinkentity( self );
	G_FreeEdict( self );
}

void cashbox_think( edict_t *ent ){

	float radius = 64.0;
	edict_t *e = NULL;

	while ((e = findradius(e, ent->s.origin, radius)) != NULL){
		if( e->client ){
			// TODO: cant attack own cashbox
			if ( ((e->client->latched_buttons|e->client->buttons) & BUTTON_ATTACK) && e->client->pers.weapon->weapmodel == WEAP_BLASTER && (e->client != ent->goalentity->client) ){
				gi.centerprintf( ent->goalentity, "%s is raiding your cashbox!", e->client->pers.netname, ent->health);
				ent->health -= 5;
			}

		}
	}

	if( ent->health < 100 ){
		ent->health += 5;
	}

	ent->nextthink = level.time + 0.2;
}


void coin_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf){

	if( other->client ){
		gi.sound(ent, CHAN_ITEM, gi.soundindex("coin/chching1.wav"), 1, ATTN_NORM, 0);

		if( Q_stricmp( ent->classname, "coin_1" ) == 0 )
			other->client->cash_in_hand += 1;
		else if( Q_stricmp( ent->classname, "coin_5" ) == 0 )
			other->client->cash_in_hand += 5;
		else if( Q_stricmp( ent->classname, "coin_10" ) == 0 )
			other->client->cash_in_hand += 10;
		else if( Q_stricmp( ent->classname, "coin_25" ) == 0 )
			other->client->cash_in_hand += 25;
		

		// aroth: this will be too frequent if there are a lot of coins. ok for now.
		if( ent->goalentity == other ){
			gi.bprintf(PRINT_MEDIUM, "%s recovered his own cash.\n", other->client->pers.netname);
		}else{
			other->client->resp.cash_stolen +=1; // track steals
			gi.bprintf(PRINT_MEDIUM, "%s accepted %s's spare change.\n", other->client->pers.netname, ent->goalentity->client->pers.netname);
			//ent->goalentity = other; // change ownership
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

		edict_t *cashbox = G_Spawn();

		//cashbox->spawnflags = DROPPED_ITEM;
		cashbox->classname = "cashbox";
		cashbox->s.modelindex = gi.modelindex("models/objects/bomb/tris.md2");
		cashbox->s.skinnum = 0;
		cashbox->mass = 400;
		cashbox->health = 120;
		cashbox->takedamage = DAMAGE_YES;
		cashbox->movetype = MOVETYPE_TOSS;
		cashbox->clipmask = MASK_SHOT;
		cashbox->solid = SOLID_BBOX;
		cashbox->owner = world;
		cashbox->die = cashbox_die;
		cashbox->goalentity = ent;
		cashbox->touch = cashbox_touch;
		cashbox->think = cashbox_think;
		cashbox->nextthink = level.time + 1;

		VectorSet( cashbox->mins, -15, -15, -15 );
		VectorSet( cashbox->maxs, 15, 15 , 15 );

		AngleVectors( client->v_angle, forward, right, NULL );

		cashbox->s.angles[1] = 0;
		cashbox->s.angles[0] = 0;
		cashbox->s.angles[2] = 0;

		VectorSet( offset, 0, 0, 0 );
		G_ProjectSource( ent->s.origin, offset, forward, right, cashbox->s.origin );
		VectorScale( forward, 200, cashbox->velocity );
		cashbox->velocity[2] = 100;

		cashbox->s.origin[2] += 40;

		gi.linkentity( cashbox );

		client->resp.cash_in_box += client->cash_in_hand;
		client->cash_in_hand = 0;
		
		gi.centerprintf(ent, "Dropped cashbox...");
		client->resp.has_cashbox = false;
	}else{
		
		float radius = 64.0;
		edict_t *e = NULL;
		while ((e = findradius(e, ent->s.origin, radius)) != NULL){
			if( strcmp( e->classname, "cashbox") == 0 && e->goalentity == ent ){
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

void throw_cash( edict_t *ent, gitem_t *it, vec3_t angle ){
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
		coin->solid = SOLID_BBOX;
		coin->takedamage = DAMAGE_YES;
		coin->clipmask = MASK_SHOT;
		coin->movetype = MOVETYPE_FLYRICOCHET;
		coin->owner = world;
		coin->goalentity = ent;
		coin->touch = coin_touch;

		AngleVectors(angle, forward, right, NULL);
		VectorSet( offset, 0, 48, 48 );
		G_ProjectSource( ent->s.origin, offset, forward, right, coin->s.origin);

		gi.linkentity (coin);
}

void Cmd_CashOut( edict_t *ent ){

	vec3_t a;
	int i;
	int total_change = 0;
	int count = ent->client->cash_in_hand;

	VectorCopy( ent->client->v_angle, a );
	
	makeChange( ent );

	total_change += ent->client->cash_25s;
	total_change += ent->client->cash_10s;
	total_change += ent->client->cash_5s;
	total_change += ent->client->cash_1s;

	gi.dprintf("TOTAL CHANGE = %d\n", total_change);

	// spit out 25s
	for( i=0; i<ent->client->cash_25s; i++ ){
		gitem_t *it = FindItem("Coin $25");
		a[1] += (360 / total_change);
		throw_cash( ent, it, a );
		ent->client->cash_in_hand -= 25;
	}

	// spit out 10s
	for( i=0; i<ent->client->cash_10s; i++ ){
		gitem_t *it = FindItem("Coin $10");
		a[1] += (360 / total_change);
		throw_cash( ent, it, a );
		ent->client->cash_in_hand -= 10;
	}

	// spit out 5s
	for( i=0; i<ent->client->cash_5s; i++ ){
		gitem_t *it = FindItem("Coin $5");
		a[1] += (360 / total_change);
		throw_cash( ent, it, a );
		ent->client->cash_in_hand -= 5;
	}

	// spit out 1s
	for( i=0; i<ent->client->cash_1s; i++ ){
		gitem_t *it = FindItem("Coin $1");
		a[1] += (360 / total_change);
		throw_cash( ent, it, a );
		ent->client->cash_in_hand -= 1;
	}




}



void makeChange( edict_t *ent ){
	int cash = ent->client->cash_in_hand;

	int c25 = 0;
	int c10 = 0;
	int c5  = 0;
	int c1  = 0;

	c25 = floor( cash / 25 );
	cash -= (c25 * 25);

	c10 = floor( cash / 10 );
	cash -= (c10 * 10);

	c5  = floor( cash / 5 );
	cash -= (c5 * 5);

	c1  = floor( cash / 1 );
	cash -= (c1 * 1);
	
	ent->client->cash_25s = c25;
	ent->client->cash_10s = c10;
	ent->client->cash_5s = c5;
	ent->client->cash_1s = c1;

	gi.dprintf("%d 25s, %d 10s, %d 5s, %d 1s\n", c25, c10, c5, c1);
}
