#include "Playback.h"
#include "PlaybackOrder.h"
#include "PhotoSlideshow.h"
#include "Platform.h"
#include "theme/ThemeRuntime.h"
#include <LittleFS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern void appShowNetworkStatus();
namespace {
constexpr uint8_t MaxPlaylistEntries=32;
constexpr uint16_t MinDurationSeconds=5, MaxDurationSeconds=3600;
const char* const PlaylistPath="/playlist.txt";
const char* const PlaylistTempPath="/playlist.tmp";
const char* const PlaylistBackupPath="/playlist.bak";
struct PlaylistEntry { char type; char id[49]; uint16_t seconds; };
PlaylistEntry g_entries[MaxPlaylistEntries];
uint8_t g_order[MaxPlaylistEntries], g_count=0;
int8_t g_cursor=-1;
bool g_shuffle=false, g_autoStart=false, g_playing=false, g_startPending=false, g_displayReady=false;
uint32_t g_dueAt=0, g_shuffleSeed=0x91E10DA5UL;
Settings* g_settings=nullptr;
WebServerClass* g_server=nullptr;

bool validThemeId(const char* id) {
  if (!id) return false;
  const size_t n = strlen(id);
  if (!n || n > 48) return false;
  for(size_t i=0;i<n;++i) { const char c=id[i]; if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_')) return false; }
  return true;
}
bool validPhotoName(const char* name) {
  if (!name) return false;
  const size_t n = strlen(name);
  if(n<5||n>=40||strcmp(name+n-4,".jpg")!=0) return false;
  for(size_t i=0;i<n-4;++i) { const char c=name[i]; if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_')) return false; }
  return true;
}
bool validEntryId(char type,const char* id) { return type=='P'?validPhotoName(id):(type=='T'&&validThemeId(id)); }
bool entryExists(const PlaylistEntry& e) {
  char path[72]; const int n=e.type=='P'?snprintf(path,sizeof(path),"/photos/%s",e.id):snprintf(path,sizeof(path),"/themes/%s.stheme",e.id);
  return n>0&&static_cast<size_t>(n)<sizeof(path)&&LittleFS.exists(path);
}
bool readLine(File& f,char* out,size_t cap) {
  if (!cap) return false;
  size_t used = 0;
  bool any = false;
  while(f.available()) { const int v=f.read(); if(v<0) break; any=true; if(v=='\n') break; if(v=='\r') continue; if(used+1>=cap) return false; out[used++]=static_cast<char>(v); }
  out[used]='\0'; return any;
}
bool readBodyLine(const String& body,size_t& pos,char* out,size_t cap) {
  if (!cap || pos > body.length()) return false;
  size_t used = 0;
  while(pos<body.length()) { const char v=body[pos++]; if(v=='\n') break; if(v=='\r') continue; if(used+1>=cap) return false; out[used++]=v; }
  out[used]='\0'; return true;
}
bool writeLine(File& f,const char* line) {
  const size_t n=strlen(line); return f.write(reinterpret_cast<const uint8_t*>(line),n)==n&&f.write(static_cast<uint8_t>('\n'))==1;
}
bool parseStoredEntry(char* line,PlaylistEntry& e) {
  char* first=strchr(line,'|'); if(!first) return false; *first++='\0';
  char* second=strchr(first,'|'); if(!second) return false; *second++='\0';
  if(line[1]||(line[0]!='P'&&line[0]!='T')||!validEntryId(line[0],first)||!*second) return false;
  char* end=nullptr; const long d=strtol(second,&end,10);
  if(!end||*end||d<MinDurationSeconds||d>MaxDurationSeconds) return false;
  e.type=line[0]; strncpy(e.id,first,sizeof(e.id)-1); e.id[sizeof(e.id)-1]='\0'; e.seconds=static_cast<uint16_t>(d); return true;
}
bool duplicateEntry(const PlaylistEntry& e) {
  for(uint8_t i=0;i<g_count;++i) if(g_entries[i].type==e.type&&strcmp(g_entries[i].id,e.id)==0) return true;
  return false;
}
void loadPlaylist() {
  g_count=0; g_cursor=-1; g_shuffle=false; g_autoStart=false;
  File f=LittleFS.open(PlaylistPath,"r"); if(!f) return;
  char line[80];
  if(!readLine(f,line,sizeof(line))||strcmp(line,"SMALLTV-PLAYLIST-1")!=0||
     !readLine(f,line,sizeof(line))||(strcmp(line,"sequence")!=0&&strcmp(line,"shuffle")!=0)) { f.close(); return; }
  g_shuffle=strcmp(line,"shuffle")==0;
  if(!readLine(f,line,sizeof(line))||(strcmp(line,"0")!=0&&strcmp(line,"1")!=0)) { f.close(); g_count=0; return; }
  g_autoStart=line[0]=='1';
  while(readLine(f,line,sizeof(line))) {
    if (!line[0]) continue;
    PlaylistEntry e{};
    if(!parseStoredEntry(line,e)||g_count>=MaxPlaylistEntries||duplicateEntry(e)) { g_count=0; g_shuffle=false; g_autoStart=false; break; }
    if (entryExists(e)) g_entries[g_count++] = e;
    yield();
  }
  f.close();
}
bool publishPlaylistFile() {
  LittleFS.remove(PlaylistBackupPath); const bool had=LittleFS.exists(PlaylistPath);
  if(had&&!LittleFS.rename(PlaylistPath,PlaylistBackupPath)) return false;
  if(!LittleFS.rename(PlaylistTempPath,PlaylistPath)) { if(had) LittleFS.rename(PlaylistBackupPath,PlaylistPath); return false; }
  LittleFS.remove(PlaylistBackupPath); return true;
}
void fillSequentialOrder(){ for(uint8_t i=0;i<g_count;++i) g_order[i]=i; }
void fillShuffleOrder(uint8_t previous) {
  g_shuffleSeed^=millis()+ESP.getChipId()+ESP.getFreeHeap(); if(!g_shuffleSeed) g_shuffleSeed=0x91E10DA5UL;
  smalltv::playback::makeShuffleOrder(g_order,g_count,g_shuffleSeed,previous);
}
bool showEntry(const PlaylistEntry& e) {
  if(e.type=='P') { smalltv::themeRuntimeStop(); smalltv::photos::stop(); return smalltv::photos::showByName(e.id); }
  smalltv::photos::stop(); std::string error;
  return smalltv::themeRuntimeSelect(e.id,g_settings->timezone,g_settings->language,error);
}
void stopInternal(bool showStatus) {
  const bool was = g_playing || g_startPending;
  g_playing = false;
  g_startPending = false;
  g_cursor = -1;
  if (!was) return;

  smalltv::themeRuntimeStop();
  smalltv::photos::stop();
  if (showStatus) {
    bool restored = false;
    if (g_settings && g_settings->activeTheme[0]) {
      std::string error;
      restored = smalltv::themeRuntimeSelect(
          g_settings->activeTheme, g_settings->timezone, g_settings->language, error);
      if (!restored) {
        Serial.printf("[playback] previous theme could not be restored: %s\\n", error.c_str());
      }
    }
    if (!restored) appShowNetworkStatus();
  }
}

bool step(int direction) {
  if(!g_count||!g_playing||!g_settings) return false;
  for(uint8_t attempt=0;attempt<g_count;++attempt) {
    if(g_cursor<0) { if(direction>0) { if(g_shuffle) fillShuffleOrder(0xFF); else fillSequentialOrder(); g_cursor=0; }
      else { if(g_shuffle) fillShuffleOrder(0xFF); else fillSequentialOrder(); g_cursor=static_cast<int8_t>(g_count-1); } }
    else if(direction>0) { if(g_cursor+1>=g_count) { const uint8_t prev=g_order[g_cursor]; if(g_shuffle) fillShuffleOrder(prev); g_cursor=0; } else ++g_cursor; }
    else { if(g_cursor==0) g_cursor=static_cast<int8_t>(g_count-1); else --g_cursor; }
    const PlaylistEntry& e=g_entries[g_order[g_cursor]];
    if(entryExists(e)&&showEntry(e)) { g_dueAt=millis()+uint32_t(e.seconds)*1000UL; return true; }
    Serial.printf("[playback] skip unavailable item: %s\n",e.id); yield();
  }
  stopInternal(true); return false;
}
bool startInternal(const char*& error) {
  error="Playback could not be started";
  if(!g_displayReady) { error="Display is not ready"; return false; }
  if(!g_count) { error="Add at least one photo or theme to the playlist"; return false; }
  if(ESP.getFreeHeap()<12UL*1024UL||platformMaxFreeBlock()<2048UL) { error="Not enough free RAM to start playback"; return false; }
  // Keep the user's manually selected theme in settings. It is restored when
  // playback stops and remains the fallback if automatic playlist start is off.
  smalltv::themeRuntimeStop();
  smalltv::photos::stop();
  g_playing = true;
  g_cursor = -1;
  if(g_shuffle) fillShuffleOrder(0xFF); else fillSequentialOrder();
  if(!step(1)) { error="No playable item found; check the installed photos and themes"; return false; }
  error=""; return true;
}
void sendReply(int code,const char* message) {
  String body; body.reserve(128); body="{\"ok\":"; body+=code<300?"true":"false"; body+=",\"message\":\""; body+=message?message:""; body+="\"}";
  g_server->send(code,"application/json",body);
}
void sendState() {
  String body; body.reserve(384+size_t(g_count)*80); body="{\"mode\":\""; body+=g_shuffle?"shuffle":"sequence";
  body+="\",\"autoStart\":"; body+=g_autoStart?"true":"false"; body+=",\"playing\":"; body+=(g_playing||g_startPending)?"true":"false";
  body+=",\"count\":"; body+=String(g_count); body+=",\"current\":"; body+=g_cursor>=0?String(g_order[g_cursor]):String(-1);
  body+=",\"entries\":[";
  for(uint8_t i=0;i<g_count;++i) { if(i) body+=','; body+="{\"type\":\""; body+=g_entries[i].type=='P'?"photo":"theme";
    body+="\",\"id\":\""; body+=g_entries[i].id; body+="\",\"seconds\":"; body+=String(g_entries[i].seconds); body+='}'; }
  body+="]}"; g_server->send(200,"application/json",body);
}
void handleSave() {
  if(g_playing||g_startPending) { sendReply(409,"Stop playback before changing the playlist"); return; }
  if(!g_server->hasArg("plain")) { sendReply(400,"Missing playlist data"); return; }
  const String body=g_server->arg("plain"); if(body.length()>3072) { sendReply(413,"Playlist data exceeds 3072 bytes"); return; }
  size_t pos=0; char line[80];
  if(!readBodyLine(body,pos,line,sizeof(line))||(strcmp(line,"sequence")!=0&&strcmp(line,"shuffle")!=0)) { sendReply(400,"Invalid playback mode"); return; }
  const bool newShuffle=strcmp(line,"shuffle")==0;
  if(!readBodyLine(body,pos,line,sizeof(line))||(strcmp(line,"0")!=0&&strcmp(line,"1")!=0)) { sendReply(400,"Invalid auto-start setting"); return; }
  const bool newAuto=line[0]=='1';
  LittleFS.remove(PlaylistTempPath); File staged=LittleFS.open(PlaylistTempPath,"w");
  if(!staged) { sendReply(500,"Could not create playlist file"); return; }
  const bool header=writeLine(staged,"SMALLTV-PLAYLIST-1")&&writeLine(staged,newShuffle?"shuffle":"sequence")&&writeLine(staged,newAuto?"1":"0");
  if(!header) { staged.close(); LittleFS.remove(PlaylistTempPath); sendReply(500,"Could not write playlist header"); return; }
  const uint8_t oldCount=g_count; const bool oldShuffle=g_shuffle, oldAuto=g_autoStart;
  g_count=0; g_shuffle=newShuffle; g_autoStart=newAuto; bool valid=true;
  while(pos<body.length()) {
    if(!readBodyLine(body,pos,line,sizeof(line))) { valid=false; break; }
    if (!line[0]) continue;
    PlaylistEntry e{};
    if(g_count>=MaxPlaylistEntries||!parseStoredEntry(line,e)||duplicateEntry(e)||!entryExists(e)) { valid=false; break; }
    char canonical[80]; snprintf(canonical,sizeof(canonical),"%c|%s|%u",e.type,e.id,static_cast<unsigned>(e.seconds));
    if(!writeLine(staged,canonical)) { valid=false; break; }
    g_entries[g_count++]=e; yield();
  }
  staged.flush(); staged.close();
  if(!valid||!publishPlaylistFile()) {
    LittleFS.remove(PlaylistTempPath); g_count=oldCount; g_shuffle=oldShuffle; g_autoStart=oldAuto; loadPlaylist();
    sendReply(400,valid?"Could not save playlist":"Invalid item, duplicate, or more than 32 entries"); return;
  }
  g_cursor=-1; sendReply(200,"Playlist saved");
}
void handleStart() {
  if(g_playing) { sendReply(200,"Playback already running"); return; }
  g_startPending=false; const char* error=nullptr;
  if(!startInternal(error)) { sendReply(503,error); return; } sendReply(200,"Playback started");
}
void handleStop(){ stopInternal(true); sendReply(200,"Playback stopped"); }
void handleNext(){ if(!g_playing){sendReply(409,"Playback is not running");return;} sendReply(step(1)?200:503,"Next item selected"); }
void handlePrevious(){ if(!g_playing){sendReply(409,"Playback is not running");return;} sendReply(step(-1)?200:503,"Previous item selected"); }
}
namespace smalltv { namespace playback {
void begin(Settings& settings,bool displayReady){g_settings=&settings;g_displayReady=displayReady;loadPlaylist();g_startPending=g_displayReady&&g_autoStart&&g_count>0;}
void webBegin(WebServerClass& server){g_server=&server;server.on("/api/playback",HTTP_GET,sendState);server.on("/api/playback/save",HTTP_POST,handleSave);server.on("/api/playback/start",HTTP_POST,handleStart);server.on("/api/playback/stop",HTTP_POST,handleStop);server.on("/api/playback/next",HTTP_POST,handleNext);server.on("/api/playback/previous",HTTP_POST,handlePrevious);}
void loop(){if(g_startPending){g_startPending=false;const char* error=nullptr;if(!startInternal(error)){Serial.printf("[playback] auto-start failed: %s\n",error?error:"unknown");stopInternal(true);}return;}if(g_playing&&static_cast<int32_t>(millis()-g_dueAt)>=0)step(1);}
void stop(){stopInternal(false);}
bool active(){return g_playing||g_startPending;}
bool autoStartRequested(){return g_startPending;}
bool advanceNow(){return g_playing&&step(1);}
bool previousNow(){return g_playing&&step(-1);}
void redraw(){if(!g_playing||g_cursor<0)return;const PlaylistEntry& e=g_entries[g_order[g_cursor]];if(e.type=='P')smalltv::photos::showByName(e.id);else smalltv::themeRuntimeInvalidate();}
} }
