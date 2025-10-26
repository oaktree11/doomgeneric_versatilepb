//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Archiving: SaveGame I/O.
//	Thinker, Ticker.
//


#include "z_zone.h"
#include "p_local.h"

#include "doomstat.h"


int	leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//



// Both the head and tail of the thinker list.
thinker_t	thinkercap;


//
// P_InitThinkers
//
void P_InitThinkers (void)
{
    thinkercap.prev = thinkercap.next  = &thinkercap;
}




//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker (thinker_t* thinker)
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
}



//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker (thinker_t* thinker)
{
  // FIXME: NOP.
  thinker->function.acv = (actionf_v)(-1);
}



//
// P_AllocateThinker
// Allocates memory and adds a new thinker at the end of the list.
//
void P_AllocateThinker (thinker_t*	thinker)
{
}
extern int _Print_thinkers;
void P_PrintAllThings(int send_coords)
{
    thinker_t* th;
    mobj_t* mo;
    int num = 0;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        // Only interested in mobjs (not other thinkers)
      //  if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mo = (mobj_t*)th;
            int x = mo->x >> FRACBITS;
            int y = mo->y >> FRACBITS;

            if (x == 3440 &&y == -3472 &&_Print_thinkers ==3) 
            // Move the Imp at 3440, -3472 in front of the player
            // This is just a test
            {
                // P_TeleportMove(mobj_t* thing, fixed_t x, fixed_t y);
                int try = P_TeleportMove(mo, 1124 << FRACBITS, -3488 << FRACBITS);
                //mo->x = 1100 << FRACBITS;
                // int y = -3600 << FRACBITS;
                uprintf("try %d\n", try);
            }

           if (send_coords){
              // uprintf1("%d,%d*", mo->x >> FRACBITS, mo->y >> FRACBITS);
              /* we are sending the thing locations to the mapviewer
              format of the thing messages is

              thing: thing_number,type,x,y

              The thing number is the position in the thinkercap list
              type is defined in the mobjinfo in info.c
              the type in the mobj_t datastructure is an index into the mobjinfo array.
              x and y positions are in world coordinates once they are divided by 16. i.e. x >> FRACBITS

              */

              uprintf1("thing: %d, %d, %d, %d\n", num, mobjinfo[mo->type].doomednum, mo->x >> FRACBITS, mo->y >> FRACBITS);
           }
            /* print the things to the console.*/
                uprintf("Enemy:#%d type %d  doomnum %d at (%d, %d, %d), HP=%d\n",num,
                    mo->type,
                     mobjinfo[mo->type].doomednum,
                    mo->x >> FRACBITS, mo->y >> FRACBITS, mo->z >> FRACBITS,
                    mo->health);
                    
            
        }
        num++;
    }
    if (send_coords)
        uprintf1("render:\n");
}

//
// P_RunThinkers
//
void P_RunThinkers (void)
{
    thinker_t*	currentthinker;

    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
	if ( currentthinker->function.acv == (actionf_v)(-1) )
	{
	    // time to remove it
	    currentthinker->next->prev = currentthinker->prev;
	    currentthinker->prev->next = currentthinker->next;
	    Z_Free (currentthinker);
	}
	else
	{
	    if (currentthinker->function.acp1)
		currentthinker->function.acp1 (currentthinker);
	}
	currentthinker = currentthinker->next;
    }
}



int have_map_viewer = 0;
//
// P_Ticker
//
extern int _Print_thinkers;
void P_Ticker(void)
{
    int		i;
    
    // run the tic
    if (paused)
	return;
		
    // pause if in menu and at least one tic has been run
    if ( !netgame
	 && menuactive
	 && !demoplayback
	 && players[consoleplayer].viewz != 1)
    {
	return;
    }
    
		
    for (i=0 ; i<MAXPLAYERS ; i++)
	if (playeringame[i])
	    P_PlayerThink (&players[i]);
			
    P_RunThinkers ();
    if (_Print_thinkers==1){
      
        P_PrintAllThings(false);
          _Print_thinkers = 0;
    }
    if (_Print_thinkers == 3)
    {

        P_PrintAllThings(false);
        _Print_thinkers = 0;
    }
    if (_Print_thinkers==2){
      
        P_PrintAllThings(true);
          _Print_thinkers = 0;
          have_map_viewer = 1; // set this variable to indicate that we assume the mapviewer is alive.
    }
    if (have_map_viewer)
    {
        /*
        TODO
         send new player coords if there has been a change in the players position

         */
        mobj_t *mo;
        thinker_t *th;
        /* the player is the first in the thinkercap list i.e. at thinkercap.next */
        mo = (mobj_t *)thinkercap.next;
    }
    int num, x, y;
    if (have_map_viewer && Get_Pos_Data(&num, &x, &y))
    {
        /* 
         TODO
         we have a move from the map viewer.

         may have to use the function P_TryMove() to move the player
         */
        
    }
    P_UpdateSpecials ();
    P_RespawnSpecials ();

    // for par times
    leveltime++;	
}
