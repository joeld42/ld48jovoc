#include <assert.h>

#include "GameState.h"
#include "GameView.h"

GameState::GameState()
{
	m_view = NULL;

	m_productionRate = DEFAULT_PRODUCTION_RATE;
}

void GameState::init()
{		
	resetLevel( -1 );
	m_gameOver = true; // start out not playing
	m_score = 0;
}

Pumpkin GameState::makeRandomPumpkin()
{
	// eventually, differnt levels will have different distributosn
	Pumpkin p;	
	p.m_type = (int)((float)rand()/(float)(RAND_MAX)*7)+1;
	return p;
}

void GameState::setView( GameView *view )
{
	m_view = view;
}

Pumpkin GameState::getPumpkin( int ndx )
{
	assert ((ndx>=0)&&(ndx<MACHINE_SLOTS));
	return m_pumpkins[ndx];
}

void GameState::dbgNextPumpkin( int ptype )
{
	Pumpkin p;
	p.m_type = ptype;
	
	m_production_queue.push_back( p );
}

void GameState::dbgClearQueue( )
{	
	m_production_queue.erase( m_production_queue.begin(), 
								m_production_queue.end() );
}

// advance the game a turn
void GameState::doTurn()
{
	Pumpkin p;

	if (m_gameOver) return;

	// burninate the pumpkin
	for (std::deque<Pumpkin>::iterator pi = m_pumpkins.begin();
		pi != m_pumpkins.end(); pi++) {

		if ((*pi).m_burning) {
			(*pi).m_burnval--;

			// all burned up
			if ((*pi).m_burnval == 0) {
				(*pi) = Pumpkin();
			}
		}
	}

	// rotate the array
	p = m_pumpkins.back();
	m_pumpkins.pop_back();
	m_pumpkins.push_front( p );

	// Handle pumpkin production
	ProducePumpkins();

	// Check if we've made a combo
	unsigned int i, future;
	bool waitForBetterCombo = false;
	Combo c;
	std::vector<int> combo_ndx;
	combo_ndx = evalBestComboValue( DEST_SLOT_NDX, c );
	if (combo_ndx.size() ) {
		
		// see if there is a future combo coming up that
		// is better
		Combo cf;
		std::vector<int> cf_ndx;
		for (future=1; future < 5; future++) {
			cf_ndx = evalBestComboValue( DEST_SLOT_NDX-future, cf );
			
			// DBG
			//if (cf_ndx.size()) {
			//	printf("Future combo %s (%d)\n", 
			//			cf.m_name.c_str(), 
			//			cf.m_value );
			//}

			if ( (cf_ndx.size()) && (cf.m_value > c.m_value)) {
				// this is better
				//printf("Waiting for better combo %s\n", cf.m_name.c_str() );
				waitForBetterCombo = true;
				break;
			}
		}

		if (!waitForBetterCombo) {

			// Notify the view about this development
			m_view->notifyAboutCombo( c, combo_ndx );

			// Remove the pumpkins
			//printf("Combo: %s : ", c.m_name.c_str() );
			for (i=0; i < combo_ndx.size(); i++) {
				//printf("%d ", combo_ndx[i] );
				m_pumpkins[combo_ndx[i]] = Pumpkin();
			}
			//printf("\n" );

			m_score += c.m_value;
			m_levelscore += c.m_value;
		}
	}
}

// Handles production and queueing
void GameState::ProducePumpkins() 
{
	// first, can the queue drop one into the machine?
	if (m_production_queue.size()) {
		if (m_pumpkins[ PRODUCTION_NDX ].m_type == EMPTY) {
			m_pumpkins[ PRODUCTION_NDX ] = m_production_queue.front();
			m_production_queue.pop_front();
		}
	}

	// produce new pumpkins
	m_turnsLeftUntilProduction--;
	if (m_turnsLeftUntilProduction == 0) {

		if (m_production_queue.size() == PRODUCTION_CAPACITY) {
			// game over man
			m_gameOver = true;
		} else {
			// Produce some produce.
			Pumpkin p = makeRandomPumpkin();
			m_production_queue.push_back( p );
			m_turnsLeftUntilProduction = m_productionRate;
		}
	}
}

void GameState::resetLevel( int lvlNdx )
{
	m_turnsLeftUntilProduction = m_productionRate;
	m_gameOver = false;
	m_center.m_type = EMPTY;
	m_levelscore = 0;
	m_score = 0;

	m_production_queue.erase( m_production_queue.begin(), m_production_queue.end() );
	m_freezer.erase( m_freezer.begin(), m_freezer.end() );
	m_pumpkins.erase( m_pumpkins.begin(), m_pumpkins.end() );

	m_combos.erase( m_combos.begin(), m_combos.end() );

	// for now.. fill the machine with
	// random pumpkins
	Pumpkin p;
	m_pumpkins.erase( m_pumpkins.begin(), m_pumpkins.end() );
	for (int i =0; i < MACHINE_SLOTS; i++)  {
		
		if (rand() < RAND_MAX / 4) {
			p = makeRandomPumpkin();
		} else {
			p = Pumpkin();
			p.m_type = EMPTY;
		}
		m_pumpkins.push_back( p );
	}

	// set up the level if one was given
	if (lvlNdx >= 0) {
		m_productionRate = m_levels[lvlNdx].m_prodrate;
		m_turnsLeftUntilProduction = m_productionRate;

		// Add the combos for this level
		for (unsigned int i=0; i < m_levels[lvlNdx].m_combos.size(); i++) {
			m_combos.push_back( m_levels[lvlNdx].m_combos[i] );
		}
	}

}

void GameState::Swap( int ndx )
{
	// need to verify ndx is valid to swap from??
	Pumpkin p;

	p = m_pumpkins[ndx];
	m_pumpkins[ndx] = m_center;
	m_center = p;
}

void GameState::Freeze( int ndx )
{
	if (m_pumpkins[ndx].m_type == EMPTY) {
		// Thaw pumpkin
		if (m_freezer.size()==0) return;
		
		m_pumpkins[ndx] = m_freezer.back();
		m_freezer.pop_back();
		
	} else {

		// put it out if its burning
		if (m_pumpkins[ndx].m_burning) {
			m_pumpkins[ndx].m_burning = false;
			m_pumpkins[ndx].m_burnval = 0;
		}

		// Freeze pumpkin
		if (m_freezer.size()==FREEZER_CAPACITY) return;

		m_freezer.push_back( m_pumpkins[ndx] );
		m_pumpkins[ndx] = Pumpkin();
	}
}

void GameState::Torch( int ndx )
{
	if (m_pumpkins[ndx].m_type != EMPTY) {
		m_pumpkins[ndx].m_burning = true;
		if (m_pumpkins[ndx].m_burnval==0) {
			m_pumpkins[ndx].m_burnval = PUMPKIN_BURN_RATE;
		}

		// Torchy costs points
		m_score -= TORCH_COST;
		m_levelscore -= TORCH_COST;
		if (m_score < 0) m_score = 0;
		if (m_levelscore < 0) m_levelscore = 0;

	}
}

// returns the hightest-point value combo available starting
// at index startndx. 
//
// returns list of indices used to make that combo
std::vector<int> GameState::evalBestComboValue( int startndx, Combo &c )
{
	int i,j;
	bool used[5], bestused[5];
	std::vector<Pumpkin> plist;

	//printf("Startndx %d: Plist is ", startndx );
	for (i=startndx; i < startndx+5; i++ ) {
		plist.push_back( m_pumpkins[i] );
		//printf("%d(%d) ", i, plist[i-startndx].m_type );
	}
	//printf( "\n" );

	int bestValue = -1;
	int bestNdx = 0;
	for (i = 0; i < m_combos.size(); i++) {

		// clear used list
		for (j=0; j < plist.size(); j++) {
			used[j] = false;
		}

		// 
		if (m_combos[i].match(plist, used)) {
			if (m_combos[i].m_value >= bestValue) {
				bestValue = m_combos[i].m_value;

				// copy used list
				for (j=0; j < plist.size(); j++) {
					bestused[j] = used[j];
				}

				bestNdx = i;
			}
		}
	}

	std::vector<int> ndx_used;
	if (bestValue >= 0 ) {
		c = m_combos[bestNdx];
		for (j=0; j < plist.size(); j++) {
			if (bestused[j]) {
				ndx_used.push_back( j+startndx );
			}
		}	
	}

	return ndx_used;
}

// access to combos
int GameState::getNumCombos( bool incSecret )
{
	int count = 0;
	for (int i=0; i < m_combos.size(); i++) {
		if ((incSecret)||(m_combos[i].m_secret==false)) {
			count++;
		}
	}
	return count;
}

Combo &GameState::getCombo( int ndx, bool incSecret )
{
	int count = 0;
	for (int i=0; i < m_combos.size(); i++) {
		if ((incSecret)||(m_combos[i].m_secret==false)) {
			if (count==ndx) return m_combos[i];
			count++;			
		}		
	}
	
	// bad index
	assert(0);
	return m_combos[0]; // not reachable.. prevents warning
}

void GameState::addCombo( Combo c )
{
	m_combos.push_back( c );
}


////////////////////////////////////////
Combo::Combo( int value, bool secret ):
	m_value( value ), m_secret( secret )
{
	m_name = "Combo";
	m_desc = "Combination of Items";
}

void Combo::addGroup( int item, int count )
{
	ComboGroup g;
	g.m_item = item;
	g.m_count = count;
	m_groups.push_back( g );
}

bool Combo::match( std::vector<Pumpkin> &plist, bool *used, int gndx )
{
	int i, targ_item;

	// everything matches
	if (gndx==m_groups.size()) return true;

	// does this group match??
	
	// Try every matching item
	int startitem, enditem;
	if (m_groups[gndx].m_item == ANY_PUMPKIN) {			
		startitem = PUMPKIN;
		enditem = PUMPKIN_RED;
	} else if (m_groups[gndx].m_item == ANY_ITEM ) {
		startitem = PUMPKIN;
		enditem = BLACKBIRD;
	} else {
		startitem = enditem = m_groups[gndx].m_item;
	}

	for (targ_item = startitem; targ_item <= enditem; targ_item++) {
		
		// can we match it?
		int count = 0;
		for ( i=0; i < 5; i++) {
			if ( (plist[i].m_type ==  targ_item) && (!used[i]) ) {
				count++;
			}
		}

		if (count >= m_groups[gndx].m_count) {
			// yes we can match it

			// use up the stuff we need
			count = 0;
			for ( i=0; i < 5; i++) {
				if ( (plist[i].m_type ==  targ_item) && (!used[i]) ) {
					used[i] = true;
					count++;
				}
				if (count==m_groups[gndx].m_count) break; // got everything we need
			}

			// see if what's left matches
			if (match( plist, used, gndx + 1)) {
				return true;
			} 

			// no.. put back the stuff we didn't use
			count = 0;
			for ( i=0; i < 5; i++) {
				if ( (plist[i].m_type ==  targ_item) && (used[i]) ) {
					used[i] = false;
					count++;
				}
				if (count==m_groups[gndx].m_count) break; // got everything we need			
			}

		}
	}

	return false;
}

void GameState::loadLevels( const std::string &filename )
{
	FILE *fp = fopen( filename.c_str(), "rt" );

	if (!fp) {
		printf ("Can't open level file %s!! This is bad...\n", 
				filename.c_str() );
		return;
	}

	Combo c;
	char line[1000], cmd[100];
	while (!feof(fp)) {
		fgets( line, 1000, fp );

		// skip comments and blank lines
		if ((line[0]=='\n') || (line[0]=='#')) continue;
		sscanf( line, "%s", cmd );

		// remove newlines
		if (line[strlen(line)-1] == '\n') {
			line[strlen(line)-1]=0;
		}

		if (!strcmp(cmd, "combo")) {
			char cname[100], secret[10], comboData[20];
			int cval;
			c = Combo();

			sscanf( line, "%*s %s %d %s %s", cname, &cval, secret, comboData );
			c.m_desc = cname;
			c.m_value = cval;
			if (!strcmp(secret, "yes")) {
				c.m_secret = true;
			} else {
				c.m_secret = false;
			}

			for (char *ch = comboData; *ch; ch +=2 ) {
				int type,  num;
				switch (*ch) {
				case 'A': type = ANY_ITEM; break;
				case 'P': type = ANY_PUMPKIN; break;
				case 'a': type = PUMPKIN; break;
				case 'b': type = PUMPKIN_ORANGE; break;	
				case 'c': type = PUMPKIN_YELLOW; break;	
				case 'd': type = PUMPKIN_RED; break;
				case 'S': type = SKULL; break;
				case 'F': type = FISH_HEAD; break;
				case 'B': type = BLACKBIRD; break;
				default:
					printf( "Unknown combo code %c. Using 'P' instead.\n", *ch );
					break;
				}

				num = ((*(ch+1)) - '1') + 1;

				c.addGroup( type, num );

				//printf("Adding combogroup %s %d - %d %d\n", c.m_desc.c_str(), 
				//		c.m_value, type, num );
			}

			c.m_name = std::string( strchr( line, ':' )+1 );

			m_allCombos.push_back( c );

		} else if (!strcmp(cmd, "level")) {
			Level l;
			char group[100], combos[2000];
			int rate;
			sscanf( line, "%*s %s %d %s", group, &rate, combos );

			l = Level();
			l.m_group = group;
			l.m_prodrate = rate;
			
			// read the title
			fgets( line, 1000, fp );
			if (line[strlen(line)-1] == '\n') {
				line[strlen(line)-1]=0;
			}
			l.m_title = line;
			
			// read the description
			while (!feof(fp)) {
				fgets( line, 1000, fp );
				if (line[0]=='\n') break;
				l.m_desc.append(  line );
			}

			// add the combos
			char *tok = strtok( combos, "," );
			while( tok ) {
				for (int i = 0; i < m_allCombos.size(); i++) {
					if (!strcmp( m_allCombos[i].m_desc.c_str(), tok)) {
						l.m_combos.push_back( m_allCombos[i] );
						break;
					}
				}
				tok = strtok( NULL, "," );
			}

			// yay
			m_levels.push_back( l );			
		}
	}	
}