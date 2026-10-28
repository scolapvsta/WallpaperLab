// UTF-8 byte positions always point to codepoint boundaries. Shared by search
// and creator text fields; clipboard data is bounded and single-line.
typedef struct {int cursor,anchor;} TextEdit;
static int EditPrev(const char* s,int at){if(at>0){at--;while(at>0&&((unsigned char)s[at]&0xc0)==0x80)at--;}return at;}
static int EditNext(const char* s,int at){if(s[at]){at++;while(s[at]&&((unsigned char)s[at]&0xc0)==0x80)at++;}return at;}
static void EditFocus(TextEdit* e,const char* s,bool select){e->cursor=strlen(s);e->anchor=select?0:e->cursor;}
static bool EditInsert(TextEdit* e,char* s,int cap,const char* value){
 char clean[8192];int n=0;for(int i=0;value&&value[i]&&n<(int)sizeof(clean)-5;){int bytes=1;unsigned char c=value[i];if(c>=0xc2&&c<=0xf4)bytes=c<0xe0?2:c<0xf0?3:4;else if(c>=128){i++;continue;}bool valid=true;for(int j=1;j<bytes;j++)if(!value[i+j]||((unsigned char)value[i+j]&0xc0)!=0x80)valid=false;if(!valid){i++;continue;}if(c>=32&&c!=127){memcpy(clean+n,value+i,bytes);n+=bytes;}i+=bytes;}clean[n]=0;
 int lo=e->anchor<e->cursor?e->anchor:e->cursor,hi=e->anchor>e->cursor?e->anchor:e->cursor,len=strlen(s),room=cap-1-(len-(hi-lo));if(n>room){n=room;while(n>0&&((unsigned char)clean[n]&0xc0)==0x80)n--;}
 if(n<0)return false;memmove(s+lo+n,s+hi,len-hi+1);memcpy(s+lo,clean,n);e->cursor=e->anchor=lo+n;return n||hi!=lo;
}
static bool EditKey(int key){return IsKeyPressed(key)||IsKeyPressedRepeat(key);}
static bool EditInput(TextEdit* e,char* s,int cap){
 int len=strlen(s);e->cursor=Clamp(e->cursor,0,len);e->anchor=Clamp(e->anchor,0,len);bool changed=false;
 bool ctrl=IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL),shift=IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT);
 if(ctrl&&IsKeyPressed(KEY_A)){e->anchor=0;e->cursor=len;}
 if(ctrl&&(IsKeyPressed(KEY_C)||IsKeyPressed(KEY_X))&&e->cursor!=e->anchor){int lo=fminf(e->cursor,e->anchor),hi=fmaxf(e->cursor,e->anchor);char copy[8192];int n=fminf(hi-lo,sizeof(copy)-1);memcpy(copy,s+lo,n);copy[n]=0;SetClipboardText(copy);if(IsKeyPressed(KEY_X))changed|=EditInsert(e,s,cap,"");}
 if(ctrl&&IsKeyPressed(KEY_V))changed|=EditInsert(e,s,cap,GetClipboardText());
 if(EditKey(KEY_LEFT)||EditKey(KEY_RIGHT)||EditKey(KEY_HOME)||EditKey(KEY_END)){
  int next=e->cursor;if(EditKey(KEY_HOME))next=0;else if(EditKey(KEY_END))next=strlen(s);else if(EditKey(KEY_LEFT)){if(!shift&&e->cursor!=e->anchor)next=fminf(e->cursor,e->anchor);else{next=EditPrev(s,next);if(ctrl){while(next>0&&s[next]==' ')next=EditPrev(s,next);while(next>0&&s[EditPrev(s,next)]!=' ')next=EditPrev(s,next);}}}else{if(!shift&&e->cursor!=e->anchor)next=fmaxf(e->cursor,e->anchor);else{next=EditNext(s,next);if(ctrl){while(s[next]&&s[next]!=' ')next=EditNext(s,next);while(s[next]==' ')next++;}}}e->cursor=next;if(!shift)e->anchor=next;
 }
 if(EditKey(KEY_BACKSPACE)||EditKey(KEY_DELETE)){if(e->cursor==e->anchor){if(EditKey(KEY_BACKSPACE))e->anchor=EditPrev(s,e->cursor);else e->anchor=EditNext(s,e->cursor);}changed|=EditInsert(e,s,cap,"");}
 int cp;while((cp=GetCharPressed())>0)if(!ctrl&&cp>=32){int bytes;const char* utf=CodepointToUTF8(cp,&bytes);char value[8]={0};memcpy(value,utf,bytes);changed|=EditInsert(e,s,cap,value);}
 return changed;
}
static int EditMeasure(const char* s,int bytes,int size){char temp[8192];int n=fminf(bytes,sizeof(temp)-1);memcpy(temp,s,n);temp[n]=0;return UiMeasure(temp,size);}
static void EditDraw(TextEdit* e,const char* s,Rectangle r,int size,bool active){
 Rectangle old=inputClip;float top=fmaxf(r.y,old.y),bottom=fminf(r.y+r.height,old.y+old.height);BeginScissorMode(fmaxf(r.x,old.x),top,fminf(r.width,old.x+old.width-r.x),fmaxf(0,bottom-top));
 int cx=EditMeasure(s,e->cursor,size),tx=r.x-(active?fmaxf(0,cx-r.width+8):0),y=r.y+(r.height-size)/2;
 if(active&&e->cursor!=e->anchor){int ax=EditMeasure(s,e->anchor,size);DrawRectangle(tx+fminf(cx,ax),y-2,abs(cx-ax),size+4,Fade(AppTheme.border,.35f));}
 UiText(s,tx,y,size,AppTheme.text);if(active&&fmod(GetTime(),1)<.55)DrawLine(tx+cx+1,y-1,tx+cx+1,y+size+2,AppTheme.text);
 EndScissorMode();if(modalDrawing)BeginScissorMode(old.x,old.y,old.width,old.height);
}
