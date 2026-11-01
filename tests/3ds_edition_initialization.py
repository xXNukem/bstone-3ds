#!/usr/bin/env python3
"""Execute original AssetsInfo, sprite and music initialization for all editions.

No retail game data or device gameplay is simulated by this test.
"""
from pathlib import Path
import re
import subprocess
import tempfile

SRC = Path(__file__).resolve().parents[1] / 'src/bstone/src'
main = (SRC / '3d_main.cpp').read_text()
ca = (SRC / 'id_ca.cpp').read_text()
header = (SRC / 'id_ca.h').read_text()

def function(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

assets_class = header[header.index('class AssetsInfo'):header.index('}; // AssetsInfo') + 2]
methods = function(ca, 'void AssetsInfo::set_version(')
methods += ca[ca.index('const std::string& AssetsInfo::get_base_path_name()'):
              ca.index('bool AssetsInfo::is_secret_level(')]
initializers = function(main, 'void initialize_sprites()') + function(main, 'void initialize_audio_constants()')
variables = sorted(set(re.findall(r'\b([A-Z][A-Z0-9_]+)\s*=', initializers)))
code = r'''
#include "bstone_game_catalog.h"
#include <cassert>
#include <stdexcept>
#include <cstdio>
#define BSTONE_THROW_STATIC_SOURCE(message) throw std::runtime_error(message)
''' + assets_class + methods + r'''
AssetsInfo assets;
AssetsInfo& get_assets_info(){return assets;}
''' + 'int ' + ','.join(variables) + ';\n' + initializers + r'''
int main(){
 for(int value=1;value<=9;++value){
  auto version=static_cast<AssetsVersion>(value);assets.set_version(version);
  initialize_sprites();initialize_audio_constants();
  if(assets.is_aog()){
   assert(assets.get_levels_per_episode()==15);
   assert(assets.get_episode_count()==(assets.is_aog_full()?6:1));
   assert(assets.get_base_path_name()=="aog");
   assert(SPR_PISTOLREADY==(assets.is_aog_sw()?489:673));
   assert(SPR_GRENADEREADY==(assets.is_aog_sw()?504:688));
   assert(THEME_MUS==16&&MENUSONG==MEETINGA_MUS&&TITLE_LOOP_MUSIC==GOLDA_MUS);
  }else{
   assert(assets.is_ps()&&assets.get_levels_per_episode()==24&&assets.get_episode_count()==1);
   assert(assets.get_base_path_name()=="ps");
   assert(SPR_PISTOLREADY==727&&SPR_GRENADEREADY==742);
   assert(SPR_BFG_WEAPON1==982&&SPR_BFG_WEAPON5==986);
   assert(THEME_MUS==20&&MENUSONG==LASTLAFF_MUS&&TITLE_LOOP_MUSIC==PLOT_MUS);
  }
  assert(assets.get_total_levels()==assets.get_episode_count()*assets.get_levels_per_episode());
 }
 std::puts("PASS: production AssetsInfo/sprite/music initialization for all nine versions, separate SW/Full/PS weapon indices, PS BFG frames and episode/level counts; retail gameplay not tested");
}
'''
with tempfile.TemporaryDirectory(prefix='bstone-editions-') as directory:
    p = Path(directory)
    (p / 'test.cpp').write_text(code)
    subprocess.run(['c++', '-std=c++14', '-fsanitize=address,undefined', '-I' + str(SRC),
                    str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
    subprocess.run([str(p / 'test')], check=True)
