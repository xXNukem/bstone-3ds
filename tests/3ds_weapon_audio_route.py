#!/usr/bin/env python3
"""Run production weapon sound selection/playback with recorded mixer requests.
This proves routing/mute/volume behavior, not audible output on hardware.
"""
from pathlib import Path
import runpy
ROOT=Path(__file__).resolve().parents[1]
h=runpy.run_path(str(ROOT/'tests/3ds_native_options.py'))
s=(ROOT/'src/bstone/src/id_sd.cpp').read_text()
agent=(ROOT/'src/bstone/src/3d_agent.cpp').read_text()
gun=agent[agent.index('void GunAttack(\n\tobjtype* ob)\n{'):agent.index('\n\tstatic const auto object_radius',agent.index('void GunAttack(\n\tobjtype* ob)\n{'))]+'\n}'
functions='\n'.join(h['function'](s,sig) for sig in [
 'void sd_play_non_positional_sfx_sound(\n\tint sound_index,',
 'void sd_play_player_sound(int sound_index, bstone::Voice& voice)\n{',
 'void sd_play_player_weapon_sound(int sound_index)\n{'])
h['run'](r'''
#include <array>
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <vector>
#define BSTONE_THROW_STATIC_SOURCE(x) throw std::runtime_error(x)
namespace bstone {
constexpr double audio_mixer_max_gain=1.;
enum class SoundType{none,adlib_sfx,pc_speaker_sfx,pcm};
struct Handle{bool valid=true;bool is_valid()const{return valid;}};
struct Voice{bool use_output_gains=false;Handle handle;double gain=1.;std::array<double,2>output_gains{{1.,1.}};};
struct AudioMixerPlaySoundParam{SoundType sound_type=SoundType::none;int sound_index=0;const void*data=nullptr;int data_size=0;bool is_looping=false,is_r3=false;};
struct Mixer{std::vector<AudioMixerPlaySoundParam>played;Handle play_sound(const AudioMixerPlaySoundParam&p){played.push_back(p);return {};}
 void enable_set_voice_output_gains(Handle,bool){} }mixer;
struct VoiceGroup{int stops=0,adds=0;double volume=1.;Voice last;
 void stop_voice(Voice&){++stops;}void stop_and_remove_voice(Voice&){}
 void add_voice(Voice&){++adds;}void set_voice_gain(const Voice&v){last=v;}
 void set_voice_output_gains(const Voice&v){last=v;}}
 group;
}
using bstone::mixer;using bstone::group;
bstone::Mixer*sd_mixer_=&mixer;bstone::VoiceGroup*sd_scene_sfx_voice_group_=&group;
bool sound=true,sd_use_voice_output_gains_=true,enhanced=false;
bool sd_is_sound_enabled(){return sound;}
enum class AudioChunkType{adlib_sfx,pc_speaker,digitized};
struct Chunk{const void*data;int data_size;AudioChunkType type;};
unsigned char samples[16]={1,2,3,4};
struct Content{Chunk chunk{samples,16,AudioChunkType::digitized};const Chunk&get_sfx_chunk(int){return chunk;}}content;
Content*audio_content_mgr=&content;
struct objtype{bstone::Voice weapon_voice;}object;
objtype*player=&object;
enum{wp_autocharge,wp_pistol,wp_burst_rifle,wp_ion_cannon,wp_grenade};
constexpr int ATKAUTOCHARGESND=23,ATKCHARGEDSND=24,ATKBURSTRIFLESND=26,ATKIONCANNONSND=11,ATKGRENADESND=87;
struct{int weapon=0;}gamestate;
void MakeAlertNoise(objtype*){}
''' + functions + gun + r'''
int main(){
 const int expected[]={23,24,26,11};
 for(bool art:{false,true})for(bool gains:{false,true})for(auto type:{AudioChunkType::adlib_sfx,AudioChunkType::pc_speaker,AudioChunkType::digitized}){
  enhanced=art;sd_use_voice_output_gains_=gains;content.chunk.type=type;sound=true;
  for(int w=0;w<4;++w){gamestate.weapon=w;const auto before=mixer.played.size();GunAttack(player);assert(mixer.played.size()==before+1);const auto&p=mixer.played.back();assert(p.sound_index==expected[w]&&p.data==samples&&p.data_size==16&&!p.is_r3&&!p.is_looping);assert(group.last.gain==1.);assert(group.last.output_gains[0]==1.&&group.last.output_gains[1]==1.);}
  const auto before=mixer.played.size();sd_play_player_weapon_sound(ATKGRENADESND);assert(mixer.played.size()==before+1&&mixer.played.back().sound_index==87);
 }
 auto count=mixer.played.size();sound=false;sd_play_player_weapon_sound(23);assert(mixer.played.size()==count);
 sound=true;player=nullptr;sd_play_player_weapon_sound(23);assert(mixer.played.size()==count);
 player=&object;content.chunk.data=nullptr;sd_play_player_weapon_sound(23);assert(mixer.played.size()==count);
 content.chunk.data=samples;sd_mixer_=nullptr;sd_play_player_weapon_sound(23);assert(mixer.played.size()==count);
 std::puts("PASS: production weapon→original sound IDs→mixer routing, original/enhanced states, AdLib/PC/PCM, output gains, disabled/null/empty guards; hardware audibility not claimed");
}
''',extra_flags=['-fsanitize=address,undefined','-fno-omit-frame-pointer'])
