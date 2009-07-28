#include <string.h>

#include <vector>
#include <algorithm>

#include <math.h>

#include "buttonlands.h"
#include "combat.h"

int combatInit = 0;
BITMAP *bgImg, *targmark;

BMHero *hero, *enemy;

BITMAP *imgNormal, *imgDead, *imgSel, *imgSm, *imgSmSwing;

// colors
unsigned int clrDiceIcon,
			 clrDiceBorder,
			 clrDiceText,

			 clrDeadIcon,
			 clrDeadBorder,
			 clrDeadText,

			 clrSelIcon,
			 clrSelBorder,
			 clrSelText;

int turn=0; // 0 player 1 enemy

int lastPass = 0;

// AI stuff
int best[5], best_targ;
int curr[5], currtarg;

int best_damage, best_reroll;
int found_move;

int show_arrow;

char best_msg[1024];

// timeout before next "step" in enemy turn;
int enemy_timeout;
int status_timeout;
char statustext[1024];
int STATUS_TIME = 140;
int ENEMY_TIME = 70;

// message
char message[1024];


void status ( const char *msg ) {

	status_timeout = STATUS_TIME;
	strcpy( statustext, msg );
}

void draw_arrow( BITMAP *targ, int x1, int y1, int x2, int y2, unsigned int col, float maxt )
{
	

	int col2,r,g,b;
	r = getr(col);
	g = getg(col);
	b = getb(col);
	col2=makecol( max(r-50,0), max(g-50,0), max(b-50,0) );

	float dirx, diry, dd;
	dirx = x2-x1; diry = y2-y1;

	// norm
	dd = sqrt( dirx*dirx + diry*diry );
	dirx /= dd; diry /= dd;

	int xx, yy;

	xx = x2 - (dirx*10);
	yy = y2 - (diry*10);
	

	for (float t = 0; t < maxt; t += (1.0/10.0) ) {
		int xp, yp;
		xp = x1 + (x2-x1)* t;
		yp = y1 + (y2-y1)* t;
		circlefill( targ, xp, yp, 10, col2 );
		circlefill( targ, xp, yp, 8, col );
	}

	// dbg
	//line( targ, x1, y1, x2, y2, col );
	//line( targ, x2, y2, xx + (diry*10), yy - (dirx*10), col );
	//line( targ, x2, y2, xx - (diry*10), yy + (dirx*10), col );
}



void ai_score( int ndx ) {
	int damage, reroll, num, shadow;

	// damge is (for now) the # of sides on the hero dice	
	damage = hero->die[ndx].sides;

	num=0; shadow = 0;

	// reroll is how "good" it is to reroll these dice
	reroll = 0;
	for (int i=0; i < 5; i++) {
		if (curr[i]) {
					
			int sides;

		
			sides = enemy->die[i].sides;			
			reroll = reroll + (sides - enemy->die[i].val);			
		}
	}

	// is this better than our best move so far??
	if ( (damage > best_damage) ||
		( (damage == best_damage) && (reroll > best_reroll) ) ) {

		found_move = 1;

		best_damage = damage;
		best_reroll = reroll;
		best_targ   = ndx;

		num = 0;
		for (int i=0; i < 5; i++) {
			best[i] = curr[i];
			if ((best[i])&&(!enemy->die[i].dead)) {
				num++;
				if (enemy->die[i].shadow) shadow = 1;
			}
		}

		// update attack message
		if (num==1) {
			if (shadow) {
				sprintf( best_msg, "%s makes a SHADOW ATTACK!", enemy->name.c_str() );
			} else {
				sprintf( best_msg, "%s makes a STRENGTH ATTACK!", enemy->name.c_str() );
			}
		} else {
			sprintf( best_msg, "%s makes a SPEED ATTACK! %d", enemy->name.c_str(), num );
		}
	}
}

void ai_evaluate()
{
	// get attack capacity
	int num=0, tot=0, shadow=0;
	for (int i=0; i < 5; i++) {
		if ((curr[i]) && (!enemy->die[i].dead)) {
			num++;
			tot += enemy->die[i].val;
			if (enemy->die[i].shadow) shadow = 1;
		}
	}

	// no dice available in this combo
	if (!num) return;

	// see who we can attack
	for (i=0; i < 5; i++) {
		
		// skip killed dice
		if (hero->die[i].dead) continue;

		if (num==1) {
			// strength or shadow
			if ( (shadow) && (tot <= hero->die[i].val)) {
				ai_score(i);
			} else if ((!shadow) && (tot >= hero->die[i].val)) {
				ai_score(i);
			}
		} else {
			// speed
			if (tot==hero->die[i].val) {
				ai_score( i );
			}
		}
	}
}

void ai_pick_dice( int ndx ) {


	if (ndx==-1) {
		
		// evaluate
		ai_evaluate();		

	} else {
		curr[ndx] = 1;
		ai_pick_dice( ndx-1 );
		curr[ndx] = 0;
		ai_pick_dice( ndx-1 );
	}
}

void ai_choose_move()
{
	best_damage = 0;
	best_reroll = 0;
	found_move = 0;
	best_targ = 0;

	for (int i=0; i < 5; i++) {
		best[i] = 0; curr[i] = 0;		
	}

	ai_pick_dice(4);

	// select the dice that we want to use
	for (i=0; i < 5; i++) {
		//if (best[i]) {
		//	enemy->die[i].selected = true;
		//} else {
			enemy->die[i].selected = false;
		//}
	}
	

	enemy_timeout = ENEMY_TIME;
	show_arrow = false;
}

void init_combat() {
	combatInit = 1;

	bgImg = load_jpeg("gamedata/bg_temp.jpg" );

	targmark = load_bitmap( "gamedata/targetmark.bmp", NULL );

	clrDiceIcon = makecol( 247, 148, 29 );
	clrDiceBorder = makecol( 147, 48, 0 );
	clrDiceText = makecol( 252, 224, 62 );

	clrSelIcon = makecol( 113, 200, 80 );
	clrSelBorder = makecol( 100, 255, 100 );
	clrSelText = makecol( 190, 255, 163 );

	clrDeadIcon = makecol( 218, 218, 218 );
	clrDeadBorder = makecol( 100,100,100 );
	clrDeadText = makecol( 50, 50, 50 );

	imgNormal = load_jpeg( "gamedata/diceicon_normal.jpg", 80 );
	imgDead   = load_jpeg( "gamedata/diceicon_dead.jpg", 80 );
	imgSel    = load_jpeg( "gamedata/diceicon_sel.jpg", 80 );

	imgSm      = load_jpeg( "gamedata/dice_sm.jpg", 80 );
	imgSmSwing = load_jpeg( "gamedata/dice_sm_swing.jpg", 80 );

}

void set_hero( BMHero *_hero )
{
	hero = _hero;
	hero->rollAll();	
}

void set_enemy( BMHero *_enemy )
{
	enemy = _enemy;
	enemy->rollAll();
}


void draw_die( BITMAP *targ, int x, int y, BMDie *die ) 
{

	unsigned int clrText, clrBorder, clrIcon;

	if (die->dead) {
		clrText = clrDeadText;
		clrBorder = clrDeadBorder;
		clrIcon = clrDeadIcon;		
	} else if (die->selected) {
		clrText = clrSelText;
		clrBorder = clrSelBorder;
		clrIcon = clrSelIcon;
		y -= 10;	
	} else {
		clrText = clrDiceText;
		clrBorder = clrDiceBorder;
		clrIcon = clrDiceIcon;
	}

	// remember loc
	die->xpos = x;
	die->ypos = y;

	// draw background circle
	if (die->dead) {
		masked_blit( imgDead, targ, 0,0, x-8, y-14, 100, 150 );

	} else if (!die->selected ) {

		masked_blit( imgNormal, targ, 0,0, x-8, y-20, 100, 100 );

	} else {

		masked_blit( imgSel, targ, 0,0, x-8, y-20, 100, 100 );
		/*
		circlefill( targ, x+40, y+40, 40, clrIcon );
		circle( targ, x+40, y+40, 40, clrBorder );

		// draw the mini circle with the max val
		circlefill( targ, x+40, y, 15, clrIcon );
		circle( targ, x+40, y, 15, clrBorder);
		*/
	} 
	// die number
	char num[100];

	if (!die->dead) {
		sprintf( num, "%d", die->val );	
		if (die->shadow) {
			alfont_set_font_size( text_font, 60 );
			alfont_textout_centre_aa( targ, text_font, num, x+40, y+15, clrText );
			int w = alfont_text_length( text_font, num ), offs = 20;
			rectfill( targ, x+40 - w/2, y+offs, x+40 + w/2, y + offs + 6, clrText );
		} else {
			// NORMAL die
			alfont_set_font_size( text_font, 80 );
			alfont_textout_centre_aa( targ, text_font, num, x+40, y+3, clrText );
		}
	}

	// die max	
	sprintf( num, "%d", die->sides );	
	alfont_set_font_size( text_font, 27 );
	alfont_textout_centre_aa( targ, text_font, num, x+40, y-12, clrText );
	if (die->shadow) {
		int w = alfont_text_length( text_font, num ), offs=2;
		rectfill( targ, x+40-w/2, y-12+offs, x+40+w/2, y-12+offs+2, clrText );
	}

	// targeting thing
	if ( (die->targetable) && (!die->dead)) {
		masked_blit( targmark, targ, 0,0, x+40-8, y+70, 16, 16 );
	}
	


	/*
	char num[100], num2[100];
	if (die->sides < SIDES_X) {
		sprintf( num, "%d", die->sides );
	} else if (die->sides==SIDES_X) {
		strcpy( num, "X" );
	} else {
		strcpy( num, "?" );
	}
	
	if (die->altsides != SIDES_NONE ) {
		if (die->altsides < SIDES_X) {
			sprintf( num2, "%d", die->altsides );
		} else if (die->altsides==SIDES_X) {
			strcpy( num2, "X" );
		} else {
			strcpy( num2, "?" );
		}
	}

	
	if (die->altsides == SIDES_NONE ) {
		alfont_set_font_size( text_font, 80 );
		alfont_textout_centre_aa( targ, text_font, num, x+40, y, clrDiceText );
	} else {
		alfont_set_font_size( text_font, 45 );
		alfont_textout_centre_aa( targ, text_font, num, x+40, y, clrDiceText );
		alfont_textout_centre_aa( targ, text_font, num2, x+40, y+40, clrDiceText );
		hline( targ, x, y+40, x+80, clrDiceText );
	}
	*/

}

void draw_portrait( BITMAP *targ, int x, int y, BMHero *hero, bool drawDice, bool equipMode )
{
	masked_blit( hero->photo, targ, 0,0, x, y, 200, 200 );

	alfont_set_font_size( title_font, 30 );

	for (int i=0; i < 5; i++) {
		for (int j=0; j < 5; j++) {
			
			alfont_textout_centre( targ, title_font, hero->name.c_str(), x+98 + i, y - 32 + j,  makecol( 0,0,0) );
		}
	}
	
	alfont_textout_centre_aa( targ, title_font, hero->name.c_str(), x+100, y - 30,  clrDiceText );

	if (drawDice) {

		char buf[50];
		for (i=0; i < 5; i++) {
			int xx, yy;
			//xx = x+160 + sin( ((float)i / 4.0) * 3.1415 )*55;
			xx = x+200;
			yy = y-20+45*i;			

			if ((!equipMode) || (hero->die[i].shadow) ) {

				masked_blit( hero->die[i].altsides?imgSmSwing:imgSm, targ,0,0, xx,yy, 50, 50 );
				
				if (hero->die[i].altsides) {
					alfont_set_font_size( text_font, 25 );
					
					sprintf( buf, "%d", hero->die[i].nrmsides );
					alfont_textout_centre_aa( targ, text_font, buf, xx+27, yy+1, makecol( 0,0,0 ) );
					alfont_textout_centre_aa( targ, text_font, buf, xx+25, yy, clrDiceText );
					
					sprintf( buf, "%d", hero->die[i].altsides );
					alfont_textout_centre_aa( targ, text_font, buf, xx+27, yy+25+1, makecol( 0,0,0 ) );
					alfont_textout_centre_aa( targ, text_font, buf, xx+25, yy+25, clrDiceText );
					
				} else {
					alfont_set_font_size( text_font, hero->die[i].shadow?30:45 );

					if (hero->die[i].shadow) yy += 10;
					
					if (hero->die[i].nrmsides != SIDES_X ) {
						sprintf( buf, "%d", hero->die[i].sides );
					} else {
						strcpy( buf, "X" );
					}
					
					alfont_textout_centre_aa( targ, text_font, buf, xx+27, yy+2, makecol( 0,0,0 ) );
					alfont_textout_centre_aa( targ, text_font, buf, xx+25, yy, clrDiceText );

					if (hero->die[i].shadow) {
						int w = (alfont_text_length( text_font, buf )-4) /2;
						
						rectfill( targ, xx+27 - w, yy +4, xx+27+w, yy, makecol(0,0,0) );
						rectfill( targ, xx+25 - w, yy +2, xx+25+w, yy-2, clrDiceText );
					}
				}
				
			} else {

				masked_blit( imgSm, targ,0,0, xx,yy, 50, 50 );

				// equip mode
				alfont_set_font_size( text_font, 45 );										
				sprintf( buf, "%d", hero->die[i].sides );
				
				alfont_textout_centre_aa( targ, text_font, buf, xx+27, yy+2, makecol( 0,0,0 ) );
				alfont_textout_centre_aa( targ, text_font, buf, xx+25, yy, clrDiceText );

				//printf("%d xx %d yy %d\n", i, xx, yy );

				if (hero->die[i].nrmsides == SIDES_X) {

					for (int j=0; allSides[j]; j++) {
						
						int offs = 70 + 60*j;
						sprintf( buf, "%d", allSides[j] );
						if (allSides[j] == hero->die[i].sides) {
							circlefill( targ, xx+offs+25, yy+25, 30, makecol(0, 255, 0 ) );
						}
						masked_blit( imgSm, targ,0,0, xx + offs,yy, 50, 50 );
						alfont_textout_centre_aa( targ, text_font, buf, xx+offs+27, yy+2, makecol( 0,0,0 ) );
						alfont_textout_centre_aa( targ, text_font, buf, xx+offs+25, yy, clrDiceText );
					}
				} else if (hero->die[i].altsides ) {
					
					for (int j=0; j < 2; j++) {
						
						int offs = 70 + 60*j;
						int num = j?hero->die[i].altsides:hero->die[i].nrmsides;
						sprintf( buf, "%d", num );
						if (num == hero->die[i].sides) {
							circlefill( targ, xx+offs+25, yy+25, 30, makecol(0, 255, 0 ) );
						}
						masked_blit( imgSm, targ,0,0, xx + offs,yy, 50, 50 );
						alfont_textout_centre_aa( targ, text_font, buf, xx+offs+27, yy+2, makecol( 0,0,0 ) );
						alfont_textout_centre_aa( targ, text_font, buf, xx+offs+25, yy, clrDiceText );
					}

				}

			}

		}
	}
}

void combat_draw( BITMAP *targ )
{
	if (!combatInit) {
		init_combat();
	}

	// draw the background
	blit( bgImg, targ, 0,0,0,0, 800, 600 );

	// draw portraits
	draw_portrait( targ, 50, 350, hero, false );
	draw_portrait( targ, 800-250, 50, enemy, false );
	

	// draw dices
	int ecount, expos;
	ecount = 0;
	for (int i=0; i < 5; i++) {
		draw_die( targ, 70+i*90, 100, &(enemy->die[i]) );
		draw_die( targ, 300+i*90, 420, &(hero->die[i]) );

		if (enemy->die[i].selected) {
			expos = 70+i*90;
			ecount++;
		}
	}

	alfont_set_font_size( text_font, 30 );	
	alfont_textout_centre( targ, text_font, message, 400, 550, turn?makecol( 0, 255, 0):makecol(255,0,0) );

	if (status_timeout) {
		int t = ((float)status_timeout / (float)STATUS_TIME) * 255.0;
		alfont_textout_centre( targ, text_font, statustext, 400, 30, makecol( t, 0, t ) );
	}
	
	// show who we're attacking
	if ((!turn) && (found_move) && (show_arrow) ) {

		
		//hline( targ, 70, 190, 70+(90*5), makecol( 255, 0, 250 ) );	

		if (ecount!=1) {
			 expos = 70+(90*2)+45;
		}
		
		for (int i=0; i < 5; i++) {
			if  ((enemy->die[i].selected) && 
				 (!enemy->die[i].dead))
			{
				draw_arrow( targ, enemy->die[i].xpos+45, 190, 
					hero->die[best_targ].xpos+40, hero->die[best_targ].ypos- 20,
					makecol( 255, 100, 50 ), 
					1.0 - ( (float)enemy_timeout / (float)ENEMY_TIME) );
			}
		}
	}

	

}

void start_combat() 
{
	hero->rollAll();
	enemy->rollAll();



	for (int i=0; i < 5; i++) {
		hero->die[i].selected = false;
		enemy->die[i].selected = false;

		hero->die[i].targetable = false;
		enemy->die[i].targetable = false;

		hero->die[i].dead = false;
		enemy->die[i].dead = false;
	}

	found_move = 0;

	// whoever has the lowest value goes first
	std::vector<int> herovals, enemyvals;
	for ( i=0; i < 5; i++) {
		herovals.push_back (hero->die[i].val );
		enemyvals.push_back (enemy->die[i].val );
	}

	std::sort( herovals.begin(), herovals.end() );
	std::sort( enemyvals.begin(), enemyvals.end() );

	for (i=0; i < 5; i++) {
		if (herovals[i] > enemyvals[i]) {
			turn = 0;
			break;
		} else if (herovals[i] < enemyvals[i]) {
			turn = 1;
			break;
		}
	}

	// if they all match, just let the player go first
	if (i==5) turn = 1;

	status( "Ready to fight!\n" );

	if (!turn) {
		ai_choose_move();
	}

}

void end_turn() {

	turn = !turn;
	for (int i=0; i < 5; i++) {
		enemy->die[i].targetable = false;
	}
	
	// re-roll selected
	for (int j=0; j < 5; j++) {
		if (hero->die[j].selected) {
			hero->die[j].selected = false;
			hero->die[j].roll();
		}
	}
	
	ai_choose_move();
}

int combat_update()
{


	// keys
	if (keypressed()) {
		int k = readkey();
			
		switch(k>>8) {			
			// DBG
#if 0
			case KEY_N:
				{
				for (int i=0; i < 5; i++) {
					hero->die[i].selected = false;
					enemy->die[i].selected = false;
				}
				turn = !turn;
				}
			break;

			case KEY_A:
				ai_choose_move();
				break;			

			case KEY_X:
				start_combat();
				break;

			case KEY_W:
				return HERO_WON;

			case KEY_D:
				return DRAW;

			case KEY_L:
				return ENEMY_WON;
#endif

			case KEY_SPACE:
				// pass
				if (turn) {
					if (lastPass) return DRAW;
					lastPass = 1;
					end_turn();
				}
		}	
	}

	// mouse click?
	static int button_state = 0;
	int pushed = 0;
	if (button_state != (mouse_b&1) ) {
		
		// clicky
		button_state = (mouse_b&1);
		if  ((button_state)&&(turn)) {

			pushed = 1;
			

		} else {
			// release
		}
	}

	// figure out help text
	if (turn) {
		int sel=0;
		int tot = 0;
		int shadow = 0;
		for (int i=0; i < 5; i++) {
			if (hero->die[i].selected) {
				sel++;
				tot += hero->die[i].val;
				if (hero->die[i].shadow) shadow = 1;
			}
		}

		if (sel==0) {
			strcpy( message, "Select dice to attack with (or SPACE to pass)." );
		} else if (sel==1) {

			if (!shadow) {
				strcpy( message, "Attack with STRENGTH ATTACK, pick more dice for SPEED ATTACK" );
			} else {
				strcpy( message, "Attack with SHADOW ATTACK, or pick more dice for SPEED ATTACK" );
			}
		} else if (sel > 1) {
			sprintf( message, "SPEED ATTACK (%d): Attack now or add dice.", tot );
		}

		// mark enemies as targetable
		for ( i=0; i < 5; i++) {		
			hero->die[i].targetable = 0;
			enemy->die[i].targetable = 0;
			if (sel==1) {
				// strength (or shadow) attack
				if (shadow) {
					if (enemy->die[i].val >= tot) enemy->die[i].targetable = 1;
				} else {
					if (enemy->die[i].val <= tot) enemy->die[i].targetable = 1;
				}
			} else {
				// speed attack
				if (enemy->die[i].val==tot)  enemy->die[i].targetable = 1;
			}

			
		}

		// press
		if (pushed) {
			for (int i=0; i < 5; i++) {
				if ( (mouse_x > hero->die[i].xpos) &&
					 (mouse_y > hero->die[i].ypos) &&
					 (mouse_x < hero->die[i].xpos+80) &&
					 (mouse_y < hero->die[i].ypos+80) &&
					 (!hero->die[i].dead)) {

					hero->die[i].selected = !hero->die[i].selected;
				}
			}

			// targeting			
			for ( i=0; i < 5; i++) {
				if ( (mouse_x > enemy->die[i].xpos) &&
					 (mouse_y > enemy->die[i].ypos) &&
					 (mouse_x < enemy->die[i].xpos+80) &&
					 (mouse_y < enemy->die[i].ypos+80) ) {


					// kill die
					if (enemy->die[i].targetable) {
						enemy->die[i].dead = true;
						enemy->die[i].selected = false;
						
						lastPass = 0;

						end_turn();

						
					}



				}
			}
		}

	} else {
		sprintf( message, "%s is making his move...", enemy->name.c_str() );
	}


	// Time
	static int last_retrace=-1;
	if (last_retrace > 0) {		
		int dt = retrace_count - last_retrace;
		if (status_timeout > 0) {
			status_timeout -= dt;
			if (status_timeout < 0) status_timeout = 0;
		}

		if ((!turn) && (enemy_timeout > 0)) {
			enemy_timeout -= dt;
			if (enemy_timeout <= 0) {
				enemy_timeout = 0;

				// do next enemy action				
				if (found_move) {

					lastPass = 0;

					// select a dice
					int done = 0;
					for (int i=0; i < 5; i++) {
						if (best[i] && !(enemy->die[i].selected)) {
							enemy->die[i].selected = true;
							done = 1;
							break;
						}
					}

					if (!done) {
						if (!show_arrow) {
							show_arrow = 1;
						} else {
							// kill die
							hero->die[best_targ].dead = 1;

							turn = 1;
							status( best_msg );
							for (int i=0; i < 5; i++) {
								if (enemy->die[i].selected) {
									printf("rolling die %d\n", i );
									enemy->die[i].roll();
									enemy->die[i].selected = false;									
								}
							}
						}
					}

					enemy_timeout = ENEMY_TIME;

				} else {
					char buff[1024];
					sprintf( buff, "%s passes.\n", enemy->name.c_str() );
					if (lastPass) return DRAW;
					lastPass = 1;
					status( buff );
					
					turn = 1;
				}
			}
		}

	}
	last_retrace = retrace_count;

	// check for a winner
	int hero_alive=0, enemy_alive=0;
	for (int i=0; i < 5; i++) {
		if (!hero->die[i].dead) hero_alive = 1;
		if (!enemy->die[i].dead) enemy_alive = 1;
	} 

	int ret;

	if ((hero_alive) && (enemy_alive)) {
		ret = STILL_FIGHTING;
	} else if (hero_alive) {
		ret = HERO_WON;
	} else {
		ret = ENEMY_WON;
	}

	return ret;
	
}