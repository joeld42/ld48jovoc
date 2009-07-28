#include <algorithm>

#include "GameState.h"
#include "GameView.h"

GameView::GameView( GameState *gamestate ) :
	m_game( gamestate )
{
	// stations
	m_stationNdxList.push_back( SWAP_NDX1 );
	m_stationNdxList.push_back( SWAP_NDX2 );
	m_stationNdxList.push_back( SWAP_NDX3 );
	m_stationNdxList.push_back( DEST_SLOT_NDX + 2 );
	m_stationNdxList.push_back( FREEZER_NDX );
	m_stationNdxList.push_back( TORCHY_NDX);

	std::sort( m_stationNdxList.begin(), m_stationNdxList.end() );

	homeStation();
	
	m_anim = true;
}

void GameView::simulate( float dt )
{
	m_turn = m_turn + (dt / TURN_TIME_SECS );
	if (m_turn > 1.0) {
		m_turn -= 1.0;

		m_game->doTurn();
	}
}


// stations are where you go to do things
void GameView::homeStation()
{
	m_station = DEST_SLOT_NDX + 2;
}

void GameView::nextStation()
{
	// find current station in the list
	int ndx = 0, i;
	for (i=0; i < m_stationNdxList.size(); i++) {
		if (m_stationNdxList[i] == m_station) {
			ndx = i;
		}
	}

	ndx++; if (ndx >= m_stationNdxList.size()) ndx = 0;
	m_station = m_stationNdxList[ndx];
}

void GameView::prevStation()
{
	// find current station in the list
	int ndx = 0, i;
	for (i=0; i < m_stationNdxList.size(); i++) {
		if (m_stationNdxList[i] == m_station) {
			ndx = i;
		}
	}

	if (ndx==0) ndx = m_stationNdxList.size()-1;
	else ndx--;
		
	m_station = m_stationNdxList[ndx];
}

void GameView::activateStation() 
{
	if ( (m_station == SWAP_NDX1)||
		 (m_station == SWAP_NDX2)||
		 (m_station == SWAP_NDX3) ) {
		
		m_game->Swap( m_station );

	} else if (m_station == FREEZER_NDX) {
		
		m_game->Freeze( m_station );

	} else if (m_station == TORCHY_NDX) {

		m_game->Torch( m_station );

	}
}