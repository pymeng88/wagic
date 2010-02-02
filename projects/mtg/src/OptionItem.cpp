#include "../include/config.h"
#include "../include/OptionItem.h"
#include <JGE.h>
#include "../include/PlayerData.h"
#include "../include/Translate.h"
#include "../include/Subtypes.h"
#include <dirent.h>
#include <stdlib.h>
#include <algorithm>

//WGuiBase

PIXEL_TYPE WGuiBase::getColor(int type){
  switch(type){
    case WGuiColor::TEXT_BODY:
    case WGuiColor::SCROLLBUTTON:
      return ARGB(255,255,255,255);
    case WGuiColor::SCROLLBAR:
      return ARGB(150,50,50,50);
    case WGuiColor::BACK_HEADER:
      return ARGB(150,80,80,80);
    default:
      if(type < WGuiColor::BACK){
        if(hasFocus())
          return ARGB(255,255,255,0);
        else
          return ARGB(255,255,255,255);
      }
      else
        return ARGB(150,50,50,50);
  }
  return ARGB(150,50,50,50);
}


void WGuiBase::renderBack(WGuiBase * it){
  if(!it) return; 
  WDecoStyled * styled = dynamic_cast<WDecoStyled*>(it);
  if(styled)
    styled->renderBack(styled->getDecorated());
  else
    subBack(it);
}


//WGuiItem
void WGuiItem::Entering(u32 key){
  mFocus = true;
}

bool WGuiItem::Leaving(u32 key){
  mFocus = false;
  return true;
}

void WGuiItem::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
  JRenderer * renderer = JRenderer::GetInstance();
  float fH = (height-mFont->GetHeight())/2;
  string trans = _(displayValue);
  float fW = mFont->GetStringWidth(trans.c_str());
  float boxW = getWidth();
  float oldS = mFont->GetScale();
  if(fW > boxW){
    mFont->SetScale(boxW/fW);
  }
  mFont->DrawString(trans,x+(width/2),y+fH,JGETEXT_CENTER);
  mFont->SetScale(oldS);
}

WGuiItem::WGuiItem(string _display, u8 _mF){
 mFlags=_mF;
 displayValue = _display;
 mFocus = false;
 width=SCREEN_WIDTH;
 height=20;
 x=0;
 y=0;
}

string WGuiItem::_(string input){
  if(mFlags & WGuiItem::NO_TRANSLATE)
    return input;
  return ::_(input);
}
bool WGuiItem::CheckUserInput(u32 key){
  if(mFocus && key == PSP_CTRL_CIRCLE){
    updateValue();
    return true;
  }
  return false; 
}

//WDecoStyled
void WDecoStyled::subBack(WGuiBase * item){
  if(!item)
    return;
  JRenderer * renderer = JRenderer::GetInstance();
  if(mStyle & DS_STYLE_BACKLESS)
    return;
  //TODO: if(mStyle & DS_STYLE_EDGED) Draw the edged box ala SimpleMenu
  else{ //Draw standard style
    WGuiSplit * split = dynamic_cast<WGuiSplit*>(item);
    if(split && split->left->Visible() && split->right->Visible()){
      if(split->left)
        renderer->FillRoundRect(split->left->getX()-2,split->getY()-2,split->left->getWidth()-6,split->getHeight(),2,split->left->getColor(WGuiColor::BACK));
      if(split->right)
        renderer->FillRoundRect(split->right->getX()-2,split->getY()-2,split->right->getWidth(),split->getHeight(),2,split->right->getColor(WGuiColor::BACK));
    }
    else{
      renderer->FillRoundRect(item->getX()-2,item->getY()-2,item->getWidth(),item->getHeight(),2,getColor(WGuiColor::BACK));
    }
  }
  
}

PIXEL_TYPE WDecoStyled::getColor(int type){
  switch(type){
    case WGuiColor::BACK:
    case WGuiColor::BACK_HEADER:
      if(mStyle & DS_COLOR_DARK)
        return ARGB(150,35,35,35);
      else if(mStyle & DS_COLOR_BRIGHT)
        return ARGB(150,80,80,80);
      else if(mStyle & DS_STYLE_ALERT)
        return ARGB(150,120,80,80);
      else
        return ARGB(150,50,50,50);        
    default:
      return WGuiBase::getColor(type);
  }
  return ARGB(150,50,50,50);
}

//WGuiHeader
void WGuiHeader::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
 
  JRenderer * renderer = JRenderer::GetInstance();
  mFont->DrawString(_(displayValue).c_str(),x+width/2,y,JGETEXT_CENTER);
}

//OptionItem
OptionItem::OptionItem( int _id,  string _displayValue): WGuiItem(_displayValue) {
 id = _id;
 mFocus=false;
}

//OptionInteger 
void OptionInteger::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
  JRenderer * renderer = JRenderer::GetInstance();

  mFont->DrawString(_(displayValue).c_str(),x,y);
  char buf[512];
  if (maxValue == 1){
    if (value)
      sprintf(buf, "%s", _("Yes").c_str());
    else
      sprintf(buf, "%s", _("No").c_str());
  }else{
    if(value == defValue && strDefault.size())
      sprintf(buf, "%s", _(strDefault).c_str());
    else
      sprintf(buf, "%i", value);
  }
  mFont->DrawString(buf,width -10 ,y,JGETEXT_RIGHT);
}

OptionInteger::OptionInteger(int _id, string _displayValue, int _maxValue, int _increment, int _defV, string _sDef, int _minValue): OptionItem(_id, _displayValue){
  defValue = _defV;
  strDefault = _sDef;
  maxValue = _maxValue;
  minValue = _minValue;
  increment = _increment;
  value = ::options[id].number;
  x = 0;
  y = 0;
}

void OptionInteger::setData(){
  if(id != INVALID_OPTION) 
    options[id] = GameOption(value);
}

//Option Select
void OptionSelect::initSelections(){
  //Find currently active bit in the list.
    for(size_t i=0;i<selections.size();i++){
      if(selections[i] == options[id].str)
        value = i;
    }
}

void OptionSelect::Entering(u32 key){
  OptionItem::Entering(key);
  prior_value = value;
}

void OptionSelect::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));

  JRenderer * renderer = JRenderer::GetInstance();
  mFont->DrawString(_(displayValue).c_str(),x,y);

  if (value < selections.size())
    mFont->DrawString(_(selections[value]).c_str(),x+width-10,y,JGETEXT_RIGHT);
  else
    mFont->DrawString(_("Unset").c_str(),x+width-10,y,JGETEXT_RIGHT);
}

void OptionSelect::setData(){
  if(id == INVALID_OPTION) return;

  if (value < selections.size())
    options[id] = GameOption(selections[value]);
}
bool OptionSelect::Selectable(){
  return (selections.size() > 1);
}

void OptionSelect::addSelection(string s){
  selections.push_back(s);
}

//OptionProfile
const string OptionProfile::DIRTESTER = "collection.dat";
OptionProfile::OptionProfile(GameApp * _app, JGuiListener * jgl) : OptionDirectory(RESPATH"/profiles", Options::ACTIVE_PROFILE, "Profile", DIRTESTER){
  app = _app;
  listener = jgl;
  height=60;
  addSelection("Default");
  sort(selections.begin(),selections.end());
  mFocus = false;
  initSelections();
  populate();
};

void OptionProfile::addSelection(string s){
  OptionDirectory::addSelection(s);

  //Check how many options... if 1, we're not selectable.
  if(selections.size() > 1)
    canSelect = true;
  else
    canSelect = false;

}
void OptionProfile::updateValue(){
 value++; 
 if (value > selections.size() - 1)
   value=0;
 
  populate();
}

void OptionProfile::Reload(){
  OptionDirectory::Reload();
  populate();
}
void OptionProfile::populate(){ 
 string temp = options[Options::ACTIVE_PROFILE].str;
 if (value >= selections.size()){ //TODO fail gracefully.
   return;
 }
 options[Options::ACTIVE_PROFILE].str = selections[value];
 PlayerData * pdata = NEW PlayerData(app->collection);
 
 int unlocked = 0, sets = setlist.size();
 std::ifstream file(options.profileFile(PLAYER_SETTINGS).c_str());
 std::string s;
  if(file){
    while(std::getline(file,s)){
      if(s.substr(0, 9) == "unlocked_")
        unlocked++;
    }
    file.close();
  }

 options[Options::ACTIVE_PROFILE] = temp;
 
 char buf[512], format[512];
 sprintf(format,"%s\n%s\n%s\n",_("Credits: %i").c_str(),_("Cards: %i").c_str(),_("Sets: %i (of %i)").c_str());
 sprintf(buf,format,pdata->credits,pdata->collection->totalCards(),unlocked,sets);
 preview = buf;

 SAFE_DELETE(pdata);
}

void OptionProfile::Render(){ 
  JRenderer * renderer = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetScale(1);
  int spacing = 2+(int)mFont->GetHeight();

  int pX, pY;
  pX = x;
  pY = y;
  char buf[512];
  if(selections[value] == "Default")
    sprintf(buf,"player/avatar.jpg");
  else
    sprintf(buf,"profiles/%s/avatar.jpg",selections[value].c_str());

  JQuad * mAvatar = resources.RetrieveQuad(buf,0,0,0,0,"temporary",RETRIEVE_NORMAL,TEXTURE_SUB_EXACT);

  if(mAvatar){
    renderer->RenderQuad(mAvatar,x,pY);
    pX += 40;
  }

  mFont->SetColor(getColor(WGuiColor::TEXT_HEADER));
  mFont->DrawString(selections[value].c_str(),pX,pY,JGETEXT_LEFT);
  mFont->SetScale(.8);
  mFont->SetColor(getColor(WGuiColor::TEXT_BODY));
  mFont->DrawString(preview.c_str(),pX,pY+spacing,JGETEXT_LEFT); 
  mFont->SetScale(1);

}
void OptionProfile::Entering(u32 key){
  mFocus = true;
  initialValue = value;
}

void OptionProfile::confirmChange(bool confirmed){
  if (initialValue >= selections.size())
   return;

  int result;

  if(confirmed)    result = value;
  else             result = initialValue;

  options[Options::ACTIVE_PROFILE] = selections[result];
  value = result;

  populate(); 
  if(listener && confirmed){
    listener->ButtonPressed(-102,5);
    initialValue = value;
  }
  return;
}
//OptionLanguage
OptionLanguage::OptionLanguage(string _displayValue) : OptionSelect(Options::LANG,_displayValue)
{
  Reload();
  initSelections();
};

void OptionLanguage::setData(){
  if(id == INVALID_OPTION) return;

  if (value < selections.size()){
    options[id] = GameOption(actual_data[value]);
    Translator::EndInstance();
    Translator::GetInstance()->init();
    Translator::GetInstance()->tempValues.clear();
  }
}
void OptionLanguage::confirmChange(bool confirmed){
  if(!confirmed)   
    value = prior_value;
  else{
    setData();
    if(Changed()){
      options[id] = GameOption(actual_data[value]);
      Translator::EndInstance();
      Translator::GetInstance()->init();
      Translator::GetInstance()->tempValues.clear();
    }
    prior_value = value;
  }
}
void OptionLanguage::Reload(){  
  struct dirent *mDit;
  DIR *mDip;

  mDip = opendir("Res/lang"); 

  while ((mDit = readdir(mDip))){
    string filename = "Res/lang/";
    filename += mDit->d_name;
    std::ifstream file(filename.c_str());
    string s;
    string lang;
    if(file){
      if(std::getline(file,s)){
        if (!s.size()){
          lang = "";
        }else{
          if (s[s.size()-1] == '\r') 
            s.erase(s.size()-1); //Handle DOS files
          size_t found = s.find("#LANG:");
          if (found != 0) lang = "";
          else lang = s.substr(6);
        }
      } 
      file.close();
    }

    if (lang.size()){
      string filen = mDit->d_name;
      addSelection(filen.substr(0,filen.size()-4),lang);
    }
  }
  closedir(mDip);
  initSelections();
}
void OptionLanguage::addSelection(string s,string show){
  selections.push_back(show);
  actual_data.push_back(s);
}
void OptionLanguage::initSelections(){
  //Find currently active bit in the list.
    for(size_t i=0;i<actual_data.size();i++){
      if(actual_data[i] == options[id].str)
        value = i;
    }
}
bool OptionLanguage::Visible(){
  if(selections.size() > 1)
    return true;
  return false;
}
bool OptionLanguage::Selectable(){
  if(selections.size() > 1)
    return true;
  return false;
}
//OptionDirectory
void OptionDirectory::Reload(){
  DIR *mDip;
  struct dirent *mDit;
  char buf[PATH_MAX];
  mDip = opendir(root.c_str());

  if (!mDip) return;

  while ((mDit = readdir(mDip))){
    sprintf(buf,"%s/%s/%s", root.c_str(), mDit->d_name, type.c_str());
    std::ifstream file(buf);
    if (!file) continue;
    file.close();
    if (find(selections.begin(), selections.end(), mDit->d_name) == selections.end())
      addSelection(mDit->d_name);
  }

  closedir(mDip);
  mDip = NULL;
  initSelections();
}

OptionDirectory::OptionDirectory(string root, int id, string displayValue, string type): OptionSelect(id, displayValue), root(root), type(type){
  DIR *mDip;
  struct dirent *mDit;
  char buf[PATH_MAX];

  mDip = opendir(root.c_str());
  if(!mDip) return;

  while ((mDit = readdir(mDip))){
    sprintf(buf,"%s/%s/%s", root.c_str(), mDit->d_name, type.c_str());
    std::ifstream file(buf);
    if (!file) continue;
    file.close();
    addSelection(mDit->d_name);
  }

  closedir(mDip);
  mDip = NULL;
  initSelections();
}

bool WGuiMenu::Leaving(u32 key){
  int nbitems = (int) items.size();
  if(key == buttonNext && currentItem < nbitems-1)
    return false;
  else if(key == buttonPrev && currentItem > 0)
    return false;

  if(currentItem >= 0 && currentItem < nbitems)
    if(!items[currentItem]->Leaving(key))
      return false;

  mFocus = false;
  return true;
}
void WGuiMenu::Entering(u32 key){  
  mFocus = true;

  //Try to force a selectable option.
  if(currentItem == -1){
    for (size_t i = 0 ; i < items.size(); i++){
      if(items[i]->Selectable()) {
        currentItem = i;
        break;
      }
    }
  }

  if(currentItem >= 0 && currentItem < (int) items.size())
    items[currentItem]->Entering(key);
  return;
}

void WGuiMenu::subBack(WGuiBase * item){
  if(!item)
    return;
  JRenderer * renderer = JRenderer::GetInstance();

  WGuiSplit * split = dynamic_cast<WGuiSplit*>(item);
  if(split && split->left->Visible() && split->right->Visible()){
    if(split->left)
      renderer->FillRoundRect(split->left->getX()-2,split->getY()-2,split->left->getWidth()-6,split->getHeight(),2,split->left->getColor(WGuiColor::BACK));
    if(split->right)
      renderer->FillRoundRect(split->right->getX()-2,split->getY()-2,split->right->getWidth(),split->getHeight(),2,split->right->getColor(WGuiColor::BACK));
  }
  else
    renderer->FillRoundRect(item->getX()-2,item->getY()-2,item->getWidth(),item->getHeight(),2,item->getColor(WGuiColor::BACK));

}
//WGuiList
WGuiList::WGuiList(string name, WDataSource * syncme): WGuiMenu(PSP_CTRL_DOWN,PSP_CTRL_UP){
  failMsg = "NO OPTIONS AVAILABLE";
  width = SCREEN_WIDTH-10;
  height = SCREEN_HEIGHT-10;
  y = 5;
  x = 5;
  mFocus = false;
  sync = syncme;
  displayValue = name;
}
void WGuiList::confirmChange(bool confirmed){
  for(size_t x=0;x<items.size();x++){
    if(!items[x])
      continue;
    items[x]->confirmChange(confirmed);
  }
}
void WGuiList::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  int listHeight=40; 
  int listSelectable=0;
  int adjustedCurrent=0;
  int start = 0, nowPos = 0, vHeight=0;
  int nbitems = (int) items.size();

  //List is empty.
  if (!items.size() && failMsg != ""){
    JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
    mFont->SetColor(getColor(WGuiColor::TEXT_FAIL));
    mFont->DrawString(_(failMsg).c_str(),x+width/2, y, JGETEXT_RIGHT);
    return;
  }

  //Force a selectable option.
  if(currentItem == -1){
    for (int i = 0 ; i < nbitems; i++){
      if(items[i]->Selectable()) {
        currentItem = i;
        if(hasFocus())
          items[currentItem]->Entering(0);
        break;
      }
    }
  }
  //Find out how large our list is, with all items and margin.
  for (int pos=0;pos < nbitems; pos++){
    listHeight+=items[pos]->getHeight()+1; //What does the +1 do exactly ?
    if(items[pos]->Selectable()){
      listSelectable++;
      if(pos < currentItem) adjustedCurrent++;
    }
  }

  //Always fill screen
  if(listHeight > SCREEN_HEIGHT)
  { 
    for (start=currentItem;start > 0; start--){
      if(!items[start]->Visible())
        continue;

      vHeight += items[start]->getHeight()+5;
      if(vHeight >= (SCREEN_HEIGHT-60)/2)
        break;
    }
    vHeight = 0;
	if(start >= 0)
    for (nowPos=nbitems;nowPos > 1; nowPos--){
       if(!items[start]->Visible())
        continue;
      vHeight += items[nowPos-1]->getHeight()+5;
    }

    if(vHeight <= SCREEN_HEIGHT-40 && nowPos < start)
      start = nowPos;
  }

  vHeight = 0;
  nowPos = 0;

  //Render items.
  if(start >= 0)
  {
    //Render current underlay.
    if(currentItem >= 0 && currentItem < nbitems && items[currentItem]->Visible())
      items[currentItem]->Underlay();

    for (int pos=0;pos < nbitems; pos++){
      if(!items[pos]->Visible())
        continue;

      if(pos < start){
        vHeight += items[pos]->getHeight() + 5;
        continue;
      }

      items[pos]->setY(y+nowPos);
      items[pos]->setX(x);
      if(listHeight > SCREEN_HEIGHT && listSelectable > 1)
        items[pos]->setWidth(width-10);
      else
        items[pos]->setWidth(width);
      nowPos += items[pos]->getHeight() + 5;
      renderBack(items[pos]);
      items[pos]->Render();
      if(nowPos > SCREEN_HEIGHT) //Stop displaying things once we reach the bottom of the screen.
        break;
    }

    //Draw scrollbar
    if(listHeight > SCREEN_HEIGHT && listSelectable > 1){
      int barPosition = y-5+((float)adjustedCurrent/listSelectable)*(SCREEN_HEIGHT-y);
      int barLength = (SCREEN_HEIGHT-y) / listSelectable;
      if(barLength < 4) barLength = 4;
      renderer->FillRect(x+width-2,y-1,2,SCREEN_HEIGHT-y,
        getColor(WGuiColor::SCROLLBAR));
      renderer->FillRoundRect(x+width-5,barPosition,5,barLength,2,
        getColor(WGuiColor::SCROLLBUTTON));
    }

    //Render current overlay.
    if(currentItem >= 0 && currentItem < nbitems && items[currentItem]->Visible())
      items[currentItem]->Overlay();
    }
}

void WGuiList::setData(){
  for (size_t i = 0; i < items.size(); i++){
    items[i]->setData();
  }
}

bool WGuiList::nextItem(){
  bool rez = WGuiMenu::nextItem();
  if(sync)
    sync->setPos(currentItem);
  return rez;
}
bool WGuiList::prevItem(){
  bool rez = WGuiMenu::prevItem();
  if(sync)
    sync->setPos(currentItem);
  return rez;
}

void WGuiList::ButtonPressed(int controllerId, int controlId){
  WGuiBase * it;

  if(!(it = Current()))
    return;

  it->ButtonPressed(controllerId,controlId);
}

const string OptionTheme::DIRTESTER = "preview.png";
OptionTheme::OptionTheme() : OptionDirectory(RESPATH"/themes", Options::ACTIVE_THEME, "Current Theme", DIRTESTER){
  addSelection("Default");
  sort(selections.begin(),selections.end());
  initSelections();
  mFocus=false; 
  bChecked = false;
}
JQuad * OptionTheme::getImage(){
  char buf[512];
  string val = selections[value];
  if(val == "Default")
    sprintf(buf,"graphics/preview.png");
  else
    sprintf(buf,"themes/%s/preview.png",val.c_str());

  return resources.RetrieveQuad(buf,0,0,0,0,"temporary",RETRIEVE_NORMAL,TEXTURE_SUB_EXACT);
}

float OptionTheme::getHeight(){
  return 130;
};
void OptionTheme::updateValue(){
  OptionDirectory::updateValue();
  bChecked = false;
}

void OptionTheme::Render(){
  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q = getImage();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT_HEADER));
  char buf[512];
  if(!bChecked){
    author = "";
    bChecked = true;
    if(selections[value] == "Default")
      sprintf(buf,RESPATH"/graphics/themeinfo.txt");
    else
      sprintf(buf,RESPATH"/themes/%s/themeinfo.txt",selections[value].c_str());
    std::ifstream file(buf);
    if(file){
      string temp;
      std::getline(file,temp);
      for(unsigned int x=0;x<17,x<temp.size();x++){
        if(isprint(temp[x])) //Clear stuff that breaks mFont->DrawString, cuts to 16 chars.
          author += temp[x];
      }
      file.close();
    }
  }
  sprintf(buf,_("Theme: %s").c_str(),selections[value].c_str());

  if(q){
    float scale = 128 / q->mHeight;
    renderer->RenderQuad(q,x, y,0,scale,scale);
  }

  mFont->DrawString(buf,x,y);
  if(bChecked && author.size()){
    mFont->SetColor(getColor(WGuiColor::TEXT_BODY));
     mFont->SetScale(.8);
    float hi = mFont->GetHeight();
    sprintf(buf,_("Artist: %s").c_str(),author.c_str());
    mFont->DrawString(buf,x,y+getHeight()-hi);
    mFont->SetScale(1);
  }
}

bool OptionTheme::Visible(){
  if(selections.size() <= 1)
    return false;

  return true;
}

void OptionTheme::confirmChange(bool confirmed){
  bChecked = false;
  if(!confirmed)   
    value = prior_value;
  else{
    setData();
    resources.Refresh(); //Update images
    prior_value = value;
  }
}
string WDecoEnum::lookupVal(int value){

  if(edef == NULL){
    int id = getId();
    if(id != INVALID_ID){
      GameOptionEnum * goEnum = dynamic_cast<GameOptionEnum*>(options.get(getId()));
      if(goEnum)
        edef = goEnum->def;
    }
  }

  if(edef){
    int idx = edef->findIndex(value);
    if(idx != INVALID_ID)
      return edef->values[idx].second;
  }

  char buf[32];
  sprintf(buf,"%d",value);
  return buf;
}

void WDecoEnum::Render()
{
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetColor(getColor(WGuiColor::TEXT));
  JRenderer * renderer = JRenderer::GetInstance();
  mFont->DrawString(_(getDisplay()).c_str(),getX(),getY());
  OptionInteger* opt = dynamic_cast<OptionInteger*>(it);
  if(opt)
    mFont->DrawString(_(lookupVal(opt->value)).c_str(), getWidth() -10, getY(), JGETEXT_RIGHT);
}

WDecoEnum::WDecoEnum(WGuiBase * _it, EnumDefinition *_edef) : WGuiDeco(_it) {edef = _edef;}
//WDecoCheat
WDecoCheat::WDecoCheat(WGuiBase * _it): WGuiDeco(_it){
  bVisible = (options[Options::ACTIVE_PROFILE].str == SECRET_PROFILE);
}
void WDecoCheat::Reload(){
  bVisible = (options[Options::ACTIVE_PROFILE].str == SECRET_PROFILE);
}
bool WDecoCheat::Visible(){
  if(bVisible && it && it->Visible())
    return true;
  return false;
}
bool WDecoCheat::Selectable(){
  if(!it || !Visible())
    return false;
  return it->Selectable();
}
//WDecoConfirm

WDecoConfirm::WDecoConfirm(JGuiListener * _listener, WGuiBase * _it): WGuiDeco(_it){
  listener = _listener;
  confirm = "Confirm";
  cancel = "Cancel";
  confirmMenu = NULL;
  bModal = false;
  mState = OP_CONFIRMED;
}

WDecoConfirm::~WDecoConfirm(){
  SAFE_DELETE(confirmMenu);
}

void WDecoConfirm::Entering(u32 key){
  setFocus(true);

  if(it)
    it->Entering(key);

  SAFE_DELETE(confirmMenu);
  mState = OP_CONFIRMED; 
  confirmMenu = NEW SimpleMenu(444, listener,Constants::MENU_FONT, 50,170);
  confirmMenu->Add(1,confirm.c_str());
  confirmMenu->Add(2,cancel.c_str());
}

bool WDecoConfirm::isModal(){
  if(bModal || (it && it->isModal()))
    return true;

  return false;
}

void WDecoConfirm::setModal(bool val){
  bModal = val;
}
void WDecoConfirm::setData(){
  if(!it)
    return;

  it->setData();
}

bool WDecoConfirm::Leaving(u32 key){
  if(!it)
    return true;

  //Choice must be confirmed.
    if(mState == OP_UNCONFIRMED){
        if(!isModal())
          setModal(true); 
        if(!it->Changed())
          mState = OP_CONFIRMED;
        else
          mState = OP_CONFIRMING;
    }
    
    if(mState == OP_CONFIRMED && it->Leaving(key)){
      setFocus(false);
      setModal(false);
      SAFE_DELETE(confirmMenu);
      return true;
    }
  
  return false;
}
bool WDecoConfirm::CheckUserInput(u32 key){
  if(hasFocus()){
    if (mState == OP_CONFIRMED && key == PSP_CTRL_CIRCLE)
      mState = OP_UNCONFIRMED;
    
    if (mState != OP_CONFIRMING && it){
      if(it->CheckUserInput(key))
        return true;
    } else if(confirmMenu && confirmMenu->CheckUserInput(key))
      return true;
  }
  return false;
}

void WDecoConfirm::Update(float dt){
  if (hasFocus()){
    if (it && mState != OP_CONFIRMING)
      it->Update(dt);
    else
      confirmMenu->Update(dt);
  }
}

void WDecoConfirm::Overlay(){
  if (confirmMenu && mState == OP_CONFIRMING)
    confirmMenu->Render();

  if(it)
    it->Overlay();
}

void WDecoConfirm::ButtonPressed(int controllerId, int controlId){
  if(controllerId == 444){
    setModal(false); 
    switch(controlId){
      case 1:
        mState = OP_CONFIRMED;      
        if(it)
          it->confirmChange(true);
        break;
      case 2:
        mState = OP_CONFIRMED;
        if(it)
          it->confirmChange(false);
        break;
    }
  }
  else
    it->ButtonPressed(controllerId,controlId);
}

WGuiButton::WGuiButton( WGuiBase* _it, int _controller, int _control, JGuiListener * jgl): WGuiDeco(_it) {
   control = _control;
   controller = _controller;
   mListener = jgl;
}

void WGuiButton::updateValue(){
  if(mListener)
    mListener->ButtonPressed(controller, control);
}

bool WGuiButton::CheckUserInput(u32 key){
  if (hasFocus() && key == PSP_CTRL_CIRCLE){
      updateValue();
      return true;
  }
  return false;
}

PIXEL_TYPE WGuiButton::getColor(int type){
  if(type == WGuiColor::BACK && hasFocus())
    return it->getColor(WGuiColor::BACK_HEADER);
  return it->getColor(type);
};

WGuiSplit::WGuiSplit(WGuiBase* _left, WGuiBase* _right) : WGuiItem("") {
  right = _right;
  left = _left;
  bRight = false;
  percentRight = 0.5f;
  if(!left->Selectable())
    bRight = true;
}
WGuiSplit::~WGuiSplit(){
  SAFE_DELETE(left);
  SAFE_DELETE(right);
}

void WGuiSplit::setData(){
  left->setData();
  right->setData();
}
void WGuiSplit::setX(float _x){
  x = _x;
  left->setX(x);
  right->setX(x+(1-percentRight)*width);
}
void WGuiSplit::setY(float _y){
  y = _y;
  left->setY(y);
  right->setY(y);
}
void WGuiSplit::setWidth(float _w){
  width = _w;
  if(right->Visible())
    left->setWidth((1-percentRight)*width);
  else
    left->setWidth(width);

  right->setWidth(percentRight*width);
}
void WGuiSplit::setHeight(float _h){
  left->setHeight(_h);
  right->setHeight(_h);
  height = _h;
}
float WGuiSplit::getHeight(){
  float lH, rH;
  lH = left->getHeight();
  rH = right->getHeight();
  if(lH > rH)
    return lH;

  return rH;
}

void WGuiSplit::Render(){
  if(right->Visible())
    right->Render();
  if(left->Visible())
    left->Render();
}

bool WGuiSplit::isModal(){
  if(bRight)
    return right->isModal();
  
  return left->isModal();
}
void WGuiSplit::setModal(bool val){
  if(bRight)
    return right->setModal(val);
  
  return left->setModal(val);
}

bool WGuiSplit::CheckUserInput(u32 key){
    if(hasFocus()){
      if (!bRight){
        if(key == PSP_CTRL_RIGHT && !isModal() 
          && right->Selectable() && left->Leaving(PSP_CTRL_RIGHT)){
          bRight = !bRight;
          right->Entering(PSP_CTRL_RIGHT);
          return true;
        }
        if(left->CheckUserInput(key))
          return true;
      }
      else 
      {
        if (key == PSP_CTRL_LEFT && !isModal() 
          && left->Selectable() && right->Leaving(PSP_CTRL_LEFT)){
          bRight = !bRight;
          left->Entering(PSP_CTRL_LEFT);
          return true;
        }
      }
      if(right->CheckUserInput(key))
        return true;
    }
    
  return false;
}

void WGuiSplit::Update(float dt){
  if(bRight)
    right->Update(dt);
  else
    left->Update(dt);
}

void WGuiSplit::Entering(u32 key){
  mFocus = true;
  if(bRight)
    right->Entering(key);
  else
    left->Entering(key);
}
bool WGuiSplit::Leaving(u32 key){

   if(bRight){
     if(right->Leaving(key)){
      mFocus = false;
      return true;
     }
   }
   else{
     if(left->Leaving(key)){
      mFocus = false;
      return true;
     }
   }

   return false;
}
void WGuiSplit::Overlay(){
  if(bRight)
    right->Overlay();
  else
    left->Overlay();
}
void WGuiSplit::Underlay(){
  if(bRight)
    right->Underlay();
  else
    left->Underlay();
}
void WGuiSplit::ButtonPressed(int controllerId, int controlId)
{
  if(bRight)
    right->ButtonPressed(controllerId, controlId);
  else
    left->ButtonPressed(controllerId, controlId);
}
void WGuiSplit::Reload(){
  left->Reload();
  right->Reload();
}
void WGuiSplit::confirmChange(bool confirmed){
  right->confirmChange(confirmed);
  left->confirmChange(confirmed);
}

//WGuiMenu
WGuiMenu::WGuiMenu(u32 next  = PSP_CTRL_RIGHT, u32 prev = PSP_CTRL_LEFT): WGuiItem(""){
  buttonNext = next;
  buttonPrev = prev;
  currentItem = -1;
}
WGuiMenu::~WGuiMenu(){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    SAFE_DELETE(*it);
  items.clear();
}
void WGuiMenu::setData(){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->setData();
};

void WGuiMenu::Reload(){
 for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->Reload();
};

void WGuiMenu::Render(){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->Render();
}
void WGuiMenu::confirmChange(bool confirmed){
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++)
    (*it)->confirmChange(confirmed);
}

void WGuiMenu::ButtonPressed(int controllerId, int controlId){
  WGuiBase * it = Current();
  if(!it) return;
  it->ButtonPressed(controllerId,controlId);
}

WGuiBase * WGuiMenu::Current(){
  if(currentItem >= 0 && currentItem < (int) items.size())
    return items[currentItem];
  return NULL;
}
void WGuiMenu::Add(WGuiBase * it){
  if(it)
    items.push_back(it);
}


bool WGuiMenu::CheckUserInput(u32 key){
  bool kidModal = false;
  bool handledInput = false;
  int nbitems = (int) items.size();
  JGE * mEngine = JGE::GetInstance();

  if(!mEngine->GetButtonState(held)) //Key isn't held down.
    held = 0;

  if(currentItem >= 0 && currentItem < nbitems)
    kidModal = items[currentItem]->isModal();

  if(!kidModal && hasFocus()){
    if (key == buttonPrev){
     held = buttonPrev;
     duration = 0;
     if(prevItem())
       return true;
    } 
    else if(held == buttonPrev && duration > 1){
     duration = .92;
     if(prevItem())
       return true;
    } 
    else if (key == buttonNext){
     held = buttonNext;
     duration = 0;
     if(nextItem())
       return true;
    }
    else if(held == buttonNext && duration > 1){
      duration = .92;      
     if(nextItem())
       return true;
    }
  }

  if(currentItem >= 0 && currentItem < nbitems)
    return items[currentItem]->CheckUserInput(key);

  return false;
}

void WGuiMenu::Update(float dt){
  int nbitems = (int) items.size();
  JGE * mEngine = JGE::GetInstance();

  if(held)
    duration += dt;
  
  if(currentItem >= 0 && currentItem < nbitems)
    items[currentItem]->Update(dt);

  for (int i = 0 ; i < nbitems; i++){
    if(i != currentItem)
      items[i]->Update(dt);
  }
}

bool WGuiMenu::nextItem(){
 int potential = currentItem;
 int nbitems = (int) items.size();

  if (potential < nbitems-1){
    potential++;
    while(potential < nbitems-1 && items[potential]->Selectable() == false)
      potential++;
    if(potential == nbitems || !items[potential]->Selectable())
      potential = -1;
    else if(potential != currentItem && items[currentItem]->Leaving(buttonNext)){
        currentItem = potential;
        items[currentItem]->Entering(buttonNext);
        return true;
    }
  }
  return false;
}

bool WGuiMenu::prevItem(){
 int potential = currentItem;

 if (potential > 0){
    potential--;
    while(potential > 0 && items[potential]->Selectable() == false)
      potential--;
    if(potential < 0 || !items[potential]->Selectable())
      potential = -1;
    else if(items[currentItem]->Leaving(buttonPrev)){
        currentItem = potential;
        items[currentItem]->Entering(buttonPrev);
        return true;
    }
 }
 return false;
}

void WGuiMenu::setModal(bool val){
  WGuiBase* c = Current();
  if(c)
    c->setModal(val);
}
bool WGuiMenu::isModal(){
  WGuiBase* c = Current();
  if(c)
    return c->isModal();

  return false;
}
//WGuiTabMenu
void WGuiTabMenu::Add(WGuiBase * it){
  if (it){
    it->setY(it->getY()+35);
    it->setHeight(it->getHeight()-35);
    WGuiMenu::Add(it);
  }
}

void WGuiTabMenu::Render(){
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  JRenderer * renderer = JRenderer::GetInstance();

  if (!items.size())
    return;

  int offset = x;
  
  for(vector<WGuiBase*>::iterator it = items.begin();it!=items.end();it++){
    int w = mFont->GetStringWidth(_((*it)->getDisplay()).c_str());
    mFont->SetColor((*it)->getColor(WGuiColor::TEXT_TAB));
    renderer->FillRoundRect(offset+5,5,w + 5,25,2,(*it)->getColor(WGuiColor::BACK_TAB));
    mFont->DrawString(_((*it)->getDisplay()).c_str(),offset+10,10);
    offset += w + 10 + 2;
  }

  WGuiBase * c = Current();
  if(c)
    c->Render();
}

void WGuiTabMenu::save(){
  confirmChange(true);
  setData();
  ::options.save();
}


//WGuiAward
void WGuiAward::Overlay(){
  JRenderer * r = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetScale(.8);
  mFont->SetColor(getColor(WGuiColor::TEXT));

  string s = details;
  if(s.size()){
    float fW = mFont->GetStringWidth(s.c_str());
    float fH = mFont->GetHeight();

    if(fH < 16)
      fH = 18;
    JQuad * button = resources.RetrieveQuad("iconspsp.png", (float)4*32, 0, 32, 32,"",RETRIEVE_NORMAL);

    r->FillRoundRect(5,10,fW+32,fH+2,2,getColor(WGuiColor::BACK));
    if(button)
      r->RenderQuad(button, 10,12,0,.5,.5);
    mFont->DrawString(s,30,16);
  }

  mFont->SetScale(1);
}
void WGuiAward::Underlay(){
  char buf[1024];
  JRenderer * r = JRenderer::GetInstance();
  JQuad * trophy = NULL;

  string n = Options::getName(id);
  if(n.size()){
    sprintf(buf,"trophy_%s.png",n.c_str()); //Trophy specific to the award
    trophy = resources.RetrieveTempQuad(buf); //Themed version...
  }
  
  if(!trophy && id >= Options::SET_UNLOCKS){
    trophy = resources.RetrieveTempQuad("trophy_set.png");  //TODO FIXME: Should look in set dir too.
  }

  if(!trophy) //Fallback to basic trophy image.
   trophy = resources.RetrieveTempQuad("trophy.png");
  
  if(trophy){
      r->RenderQuad(trophy, 0, SCREEN_HEIGHT-trophy->mHeight);
  }

}
void WGuiAward::Render(){
  GameOptionAward * goa = dynamic_cast<GameOptionAward*>(&options[id]);
  
  if(!goa)
    return;

  JRenderer * renderer = JRenderer::GetInstance();
  JLBFont * mFont = resources.GetJLBFont(Constants::OPTION_FONT);
  mFont->SetScale(1);
  mFont->SetColor(getColor(WGuiColor::TEXT));

  float myX = x;
  float myY = y;
  float fH = mFont->GetHeight();
  float fM = fH / 5; //Font Margin is 20% font height

  renderer->FillRoundRect(x-fM/2,y-fM,getWidth()-fM,fH-fM,fM,getColor(WGuiColor::BACK_TAB));
  myX += fM;

  mFont->DrawString(_(displayValue).c_str(),myX,myY,JGETEXT_LEFT);
  myY += fH + 3*fM;

  mFont->SetScale(.75);
  fH = mFont->GetHeight();
  if(text.size()){
    mFont->DrawString(_(text).c_str(),myX,myY,JGETEXT_LEFT);
    myY+=fH + fM;
  }
  string s = goa->menuStr();
  if(s.size()){
    mFont->DrawString(s.c_str(),myX,myY,JGETEXT_LEFT);
    myY+=fH + fM;
  }
  setHeight(myY-y);
  mFont->SetScale(1);

}

WGuiAward::WGuiAward(int _id, string name, string _text, string _details): WGuiItem(name){
  id = _id;
  text = _text;
  height = 60;
  details = _details;
}
WGuiAward::~WGuiAward(){
  GameOptionAward * goa = dynamic_cast<GameOptionAward*>(&options[id]);
  if(goa) 
    goa->setViewed(true);
}
bool WGuiAward::Visible(){
  //WGuiAward is only visible when it's tied to an already acchieved award.
  GameOptionAward * goa = dynamic_cast<GameOptionAward*>(&options[id]);
  if(!goa || !goa->number)
    return false; 
  return true;
}

//WGuiImage
WGuiImage::WGuiImage(WDataSource * wds, float _w, float _h, int _margin): WGuiItem("") {
  imgW = _w;
  imgH = _h;
  margin = _margin;
  source = wds;
}

void WGuiImage::imageScale(float _w, float _h){
  imgH = _h;
  imgW = _w;
}

float WGuiImage::getHeight(){
  JQuad * q = NULL;

  if(imgH == 0 ){
    if(source && (q = source->getImage())) //Intentional assignment.
      return MAX(height,q->mHeight+(2*margin));
  }
    
  return MAX(height,imgH+(2*margin));
}

void WGuiImage::Render(){
  if(!source)
    return;

  JRenderer * renderer = JRenderer::GetInstance();
  JQuad * q = source->getImage();
  if(q){
    float xS = 1, yS = 1;
    if(imgH != 0 && q->mHeight != 0)
      yS = imgH / q->mHeight;
    if(imgW != 0 && q->mWidth != 0)
      xS = imgW / q->mWidth;

    renderer->RenderQuad(q,x+margin, y+margin,0,xS,yS);
  }
}

WGuiCardImage::WGuiCardImage(WDataSource * wds, int _offset) : WGuiImage(wds){
  offset = _offset;
};

void WGuiCardImage::Render(){
  if(!source)
    return;
  
  JRenderer * renderer = JRenderer::GetInstance();
  MTGCard * c = source->getCard();
  if(!c)
    return;

  Pos p(x+margin, y+margin, 1,0,255);

  int oldpos = source->getPos();
  
  if(offset){
    source->setPos(oldpos+offset);   
    c = source->getCard();
  }

  JQuad * q = source->getImage();
  if(!q || options[Options::DISABLECARDS].number)
    CardGui::alternateRender(c,p);
  else
    CardGui::RenderBig(c,p);

  if(offset)
    source->setPos(oldpos);
}
//WSrcImage
JQuad * WSrcImage::getImage(){
  return resources.RetrieveTempQuad(filename);
}

WSrcImage::WSrcImage(string s){
  filename = s;
}

bool WCSortAlpha::operator()(const MTGCard*l, const MTGCard*r){
  if(!l || !r || !l->data || !r->data)
    return false;
  string ln = l->data->getLCName();
  string rn = r->data->getLCName();
  return (ln < rn);
}
bool WCSortCollector::operator()(const MTGCard*l, const MTGCard*r){
  if(!l || !r || !l->data || !r->data)
    return false;

  int lc, rc;
  lc = l->data->countColors(); rc = r->data->countColors();
  if(lc == 0) lc = 999;
  if(rc == 0) rc = 999;

  int isW = (int)l->data->hasColor(Constants::MTG_COLOR_WHITE) - (int) r->data->hasColor(Constants::MTG_COLOR_WHITE);
  int isU = (int)l->data->hasColor(Constants::MTG_COLOR_BLUE) - (int) r->data->hasColor(Constants::MTG_COLOR_BLUE);
  int isB = (int)l->data->hasColor(Constants::MTG_COLOR_BLACK) - (int) r->data->hasColor(Constants::MTG_COLOR_BLACK);
  int isR = (int)l->data->hasColor(Constants::MTG_COLOR_RED) - (int) r->data->hasColor(Constants::MTG_COLOR_RED);
  int isG = (int)l->data->hasColor(Constants::MTG_COLOR_GREEN) - (int) r->data->hasColor(Constants::MTG_COLOR_GREEN);
  int isArt = (int)l->data->hasType(Subtypes::TYPE_ARTIFACT) - (int) r->data->hasType(Subtypes::TYPE_ARTIFACT);
  int isLand = (int)l->data->hasType(Subtypes::TYPE_LAND) - (int) r->data->hasType(Subtypes::TYPE_LAND);

  //Nested if hell. TODO: Farm these out to their own objects as a user-defined filter/sort system.
  if(!isLand){
    int isBasic = (int)l->data->hasType("Basic") - (int) r->data->hasType("Basic");
    if(!isBasic){
      if(!isArt){
        if(lc == rc){
          if(!isG){
            if(!isR){
              if(!isB){
                if(!isU){
                  if(!isW){
                    string ln = l->data->getLCName();
                    string rn = r->data->getLCName();
                    if(ln.substr(0,4) == "the ")
                      ln = ln.substr(4);
                    if(rn.substr(0,4) == "the ")
                      rn = rn.substr(4);
                    return (ln < rn);
                  }
                  return (isW < 0);
                }
                return (isU < 0);
              }
              return (isB < 0);
            }
            return (isR < 0);
          }
          return (isG < 0);
        }
        return (lc < rc);
      }
      return (isArt < 0);
    }
    else return(isBasic < 0);
  }
  return (isLand < 0);
}
//WSrcMTGSet
WSrcMTGSet::WSrcMTGSet(int setid, float delay){
  MTGAllCards * ac = GameApp::collection;
  map<int,MTGCard*>::iterator it;
  mDelay = delay;
  mLastInput = 0;

  for(it = ac->collection.begin();it != ac->collection.end();it++){
    if(it->second->setId == setid && it->second->getId() >= 0)
      cards.push_back(it->second);
  }

  std::sort(cards.begin(),cards.end(),WCSortCollector());

  currentCard = -1;
  if(cards.size())
    currentCard = 0;
}
JQuad * WSrcMTGSet::getImage(){
#if defined WIN32 || defined LINUX //Loading delay only on PSP.
#else
  if(mDelay && mLastInput < mDelay){
      return resources.RetrieveCard(getCard(),RETRIEVE_EXISTING);
  }
#endif
  
  return resources.RetrieveCard(getCard());
}
void WSrcMTGSet::Update(float dt){
  mLastInput += dt;
}
MTGCard * WSrcMTGSet::getCard(){
  int size = (int) cards.size();
  if(!size || currentCard < 0 || currentCard >= size)
   return NULL;

 return cards[currentCard];
}

bool WSrcMTGSet::next(){
  if(currentCard < (int) cards.size())
    currentCard++;
  else
    return false;

  mLastInput = 0;
  return true;
}

bool WSrcMTGSet::prev(){
  if(currentCard > 0)
    currentCard--;
  else
    return false;

  mLastInput = 0;
  return true;
}

bool WSrcMTGSet::setPos(int pos){
  if(pos < 0 || pos >= (int) cards.size())
    return false;

  currentCard = pos;
  mLastInput = 0;
  return true;
}

bool WSrcMTGSet::thisCard(int mtgid){
  for(size_t t=0;t<cards.size();t++){
    if(cards[t] && cards[t]->getId() == mtgid){
      currentCard = (int) t;
      return true;
    }
  }
  return false;
}
