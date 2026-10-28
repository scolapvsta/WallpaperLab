"""Read bounded entries from local Workshop PKGV containers for opt-in tests."""
import struct,pathlib

def read_asset(project,name):
 project=pathlib.Path(project);rel=pathlib.Path(name)
 if rel.is_absolute()or'..'in rel.parts:raise ValueError('Invalid asset path')
 direct=project/rel
 if direct.is_file():return direct.read_bytes()
 with(project/'scene.pkg').open('rb')as f:
  def u32():return struct.unpack('<I',f.read(4))[0]
  def text():
   n=u32()
   if n>4096:raise ValueError('Invalid entry name')
   return f.read(n).decode()
  if not text().startswith('PKGV'):raise ValueError('Unknown package')
  count=u32()
  if count>100000:raise ValueError('Invalid package index')
  match=None
  for _ in range(count):
   entry=text();offset=u32();size=u32()
   if entry==name:match=(offset,size)
  if not match:raise FileNotFoundError(name)
  if match[1]>128*1024*1024:raise ValueError('Test asset exceeds 128 MiB')
  base=f.tell();f.seek(base+match[0]);data=f.read(match[1]);assert len(data)==match[1];return data
