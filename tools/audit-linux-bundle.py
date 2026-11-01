#!/usr/bin/env python3
"""Read-only dependency/ABI audit. This is not a physical Steam Deck test."""
import argparse, json, os, pathlib, re, subprocess, sys, datetime
p=argparse.ArgumentParser();p.add_argument('appdir',type=pathlib.Path);p.add_argument('--output',default='COMPATIBILITY_AUDIT.json');a=p.parse_args()
root=a.appdir.resolve(); elf=[]; seen=set(); failures=[]
for f in root.rglob('*'):
    if not f.is_file():continue
    real=f.resolve()
    if real in seen:continue
    seen.add(real)
    if not real.is_relative_to(root):failures.append('External symlink: '+str(f));continue
    with f.open('rb') as stream:magic=stream.read(4)
    if magic==b'\x7fELF':elf.append(f)
needed=set(); bundled=set(); records=[]; max_glibc=(0,0); max_glibcxx=(0,0,0)
env=dict(os.environ);env['LD_LIBRARY_PATH']=str(root/'usr/lib')
for f in elf:
    header=subprocess.check_output(['readelf','-h',str(f)],text=True)
    if 'Advanced Micro Devices X86-64' not in header:failures.append('Non-x86-64 ELF: '+str(f.relative_to(root)))
    dynamic=subprocess.check_output(['readelf','-d',str(f)],text=True)
    deps=re.findall(r'\(NEEDED\).*\[(.*?)\]',dynamic);needed.update(deps)
    soname=re.findall(r'\(SONAME\).*\[(.*?)\]',dynamic);bundled.update(soname)
    bundled.add(f.name)
    versions=subprocess.check_output(['readelf','-V',str(f)],text=True)
    needs=versions.partition('Version needs section')[2]
    for v in re.findall(r'Name: GLIBC_(\d+\.\d+(?:\.\d+)?)',needs):max_glibc=max(max_glibc,tuple(map(int,v.split('.'))))
    for v in re.findall(r'Name: GLIBCXX_(\d+\.\d+\.\d+)',needs):max_glibcxx=max(max_glibcxx,tuple(map(int,v.split('.'))))
    load=subprocess.run(['ldd',str(f)],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env).stdout
    missing=[line.strip() for line in load.splitlines() if 'not found' in line]
    if missing:failures.append(str(f.relative_to(root))+': '+', '.join(missing))
    records.append({'path':str(f.relative_to(root)),'needed':deps})
# SteamOS should supply its complete matching graphics stack and ALSA configuration.
platform=re.compile(r'^lib(?:GL[^/]*|EGL|OpenGL|vulkan|gbm|drm[^/]*|va(?:-[^/]*)?|vdpau|asound|pipewire[^/]*)\.so')
leaked=sorted(x for x in bundled if platform.match(x))
if leaked:failures.append('Bundled platform stack should come from SteamOS: '+', '.join(leaked))
required=['libQt6Core.so.6','libQt6Qml.so.6','libQt6Quick.so.6','libSDL2-2.0.so.0','libavcodec.so.58','libopus.so.0','libssl.so.3']
for item in required:
    if item not in bundled:failures.append('Missing bundled framework: '+item)
for plugin in ['usr/plugins/platforms/libqxcb.so','usr/plugins/platforms/libqwayland-egl.so']:
    if not (root/plugin).exists():failures.append('Missing display backend: '+plugin)
if not (root/'usr/qml/QtQuick/Controls').is_dir():failures.append('Qt Quick Controls QML missing')
if max_glibc>(2,35):failures.append('ELF requires glibc newer than the declared Ubuntu 22.04 baseline')
report={'generated_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'target':'SteamOS 3.9.5 stable, checked 2026-11-01','reference':'https://www.steamdeck.com/en/news','elf_count':len(elf),'max_required_glibc':'.'.join(map(str,max_glibc)),'max_required_glibcxx':'.'.join(map(str,max_glibcxx)),'bundled_frameworks':required,'system_sonames':sorted(needed-bundled),'failures':failures,'elf_dependencies':records,'scope':'All packaged ELF dependency resolution checked on build OS. SteamOS repository ABI comparison and X11/Wayland smoke tests are additional checks. Physical GPU/controller, SteamOS boot and gamescope integration require on-device testing.'}
pathlib.Path(a.output).write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:v for k,v in report.items() if k!='elf_dependencies'},indent=2))
sys.exit(bool(failures))
