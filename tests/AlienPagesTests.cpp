#include "all_aliens_paging.h"
#include <stdexcept>
static void Check(bool ok){if(!ok)throw std::runtime_error("Alien page input regression");}
int main(){
  ben10::AlienPages pages;
  Check(pages.Poll(false,true,255,255)==-1);
  Check(pages.Poll(true,true,0,0)==0);
  Check(pages.Poll(true,true,255,0)==0);
  Check(pages.Poll(true,true,255,255)==1);
  for(int i=0;i<100;++i)Check(pages.Poll(true,true,255,255)==1);
  Check(pages.Poll(true,true,255,0)==1);
  Check(pages.Poll(true,true,255,255)==2);
  Check(pages.Poll(true,true,255,0)==2);
  Check(pages.Poll(true,true,255,255)==0);
  Check(pages.Poll(true,true,0,255)==0);
  Check(pages.Poll(true,true,255,255)==0);
  Check(pages.Poll(true,true,255,0)==0);
  Check(pages.Poll(true,true,255,255)==1);
  Check(pages.Poll(true,false,255,255)==0);
  Check(pages.Poll(true,true,255,255)==0);
  Check(pages.Poll(false,true,255,0)==-1);
  Check(pages.Poll(true,true,255,255)==0);
}
