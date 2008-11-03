#include "../include/debug.h"
#include "../include/CardDisplay.h"
#include "../include/CardGui.h"
#include "../include/TargetChooser.h"
#include "../include/MTGGameZones.h"

CardDisplay::CardDisplay():PlayGuiObjectController(0, GameObserver::GetInstance()){
	tc= NULL;
	listener = NULL;
	nb_displayed_items = 7;
	start_item = 0;
	x= 0;
	y= 0;
}

CardDisplay::CardDisplay(int id, GameObserver* _game, int _x, int _y, JGuiListener * _listener, TargetChooser * _tc, int _nb_displayed_items ):PlayGuiObjectController(id, _game), x(_x), y(_y){
	tc= _tc;
	listener = _listener;
	nb_displayed_items = _nb_displayed_items;
	start_item = 0;
}


void CardDisplay::AddCard(MTGCardInstance * _card){
	CardGui * card = NEW CardGui(mCount,  _card, 40, x + 5 + (mCount - start_item) * 30, y + 5, (mCount == 0));
	Add(card);
}


void CardDisplay::init(MTGGameZone * zone){
	resetObjects();
	start_item = 0;
	for (int i= 0; i< zone->nb_cards; i++){
		AddCard(zone->cards[i]);
	}
}

void CardDisplay::rotateLeft(){
	if (start_item==0) return;
	for (int i= 0; i<mCount; i++){
		CardGui * cardg = (CardGui *)mObjects[i];
		cardg->x+=30;
	}
	start_item --;
}

void CardDisplay::rotateRight(){
	if (start_item==mCount-1) return;
	for (int i= 0; i<mCount; i++){
		CardGui * cardg = (CardGui *)mObjects[i];
		cardg->x-=30;
	}
	start_item ++;
}


void CardDisplay::CheckUserInput(float dt){
	if (mEngine->GetButtonClick(PSP_CTRL_CROSS))
	{
			if (listener != NULL)
			{
				listener->ButtonPressed(mId, 0);
				return;
			}
	}


		if (!mCount)
		return;

	if (mEngine->GetButtonClick(mActionButton))
	{
		if (mObjects[mCurr] && mObjects[mCurr]->ButtonPressed()){
			CardGui * cardg = (CardGui *)mObjects[mCurr];
			if (tc)
			{
				tc->toggleTarget(cardg->card);
				return;
			}else{
				if (game) game->ButtonPressed(mId, cardg);
				return;
			}
		}
	}
	

	if (mEngine->GetButtonState(PSP_CTRL_LEFT))
	{
		if (KeyRepeated(PSP_CTRL_LEFT, dt))
		{
			int n = mCurr;
			n--;
			if (n<start_item){
				if (n< 0){n = 0;}
				else{ rotateLeft();}
			}
			if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_LEFT)){
				mCurr = n;
				mObjects[mCurr]->Entering();
			}
		}
	}
	else if (mEngine->GetButtonState(PSP_CTRL_RIGHT))
	{
		if (KeyRepeated(PSP_CTRL_RIGHT, dt))
		{
			int n = mCurr;
			n++;
			if (n>= mCount){n = mCount-1;}
			if (n>= start_item + nb_displayed_items){
				rotateRight();
			}
			if (n != mCurr && mObjects[mCurr] != NULL && mObjects[mCurr]->Leaving(PSP_CTRL_RIGHT)){
				mCurr = n;
				mObjects[mCurr]->Entering();
			}
		}
	}

	else{
		mLastKey = 0;
	}

}


void CardDisplay::Render(){
	
	JRenderer * r = JRenderer::GetInstance();
	r->DrawRect(x,y,nb_displayed_items * 30 + 20, 50, ARGB(255,255,255,255));
if (!mCount) return;
	for (int i = start_item; i< start_item + nb_displayed_items && i < mCount; i++){
		if (mObjects[i]){			
			mObjects[i]->Render();
			if (tc){
				CardGui * cardg = (CardGui *)mObjects[i];
				if( tc->alreadyHasTarget(cardg->card)){
					r->DrawCircle(cardg->x + 5, cardg->y+5,5, ARGB(255,255,0,0));
				}else if (!tc->canTarget(cardg->card)){
					r->FillRect(cardg->x,cardg->y,30,40,ARGB(200,0,0,0));
				}
			}
		}
	}
	if (mObjects[mCurr]){			
		mObjects[mCurr]->Render();
	}
}

DefaultTargetDisplay::DefaultTargetDisplay(int id, GameObserver* _game, int _x, int _y,JGuiListener * _listener, int _nb_displayed_items ):CardDisplay(id, _game,  _x,  _y, _listener, NULL, _nb_displayed_items ){
	tc = NEW TargetChooser();
}

DefaultTargetDisplay::~DefaultTargetDisplay(){
	SAFE_DELETE(tc);
}