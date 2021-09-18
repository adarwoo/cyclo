#!/usr/bin/env python3
from PIL import Image
import sys
import json
import textwrap
import re
from pathlib import Path

VALID_ID = re.compile(r"\w(\w_)*")


class ParserException(Exception):
   pass


class Resource:
   def __init__(self, id, path, node):
      self.id = id
      self.inc = []
      self.src = []
      self.source_file = None

      # Get the resource file
      source_file = node.get("source", None)

      if not source_file:
         raise ParserException(f"Missing 'source' in resource ID {self.id}")

      source_file = Path(source_file)

      if not source_file.is_absolute():
         source_file = path / Path(source_file)

      if not source_file.exists():
         raise ParserException(f"No such file: {str(source_file)}")

      self.source_file = str(source_file)

   def build(self, node):
      pass

   def get_code(self, source=False):
      return self.src if source else self.inc

class Bitmap(Resource):
   def build(self, node):
      im = Image.open(str(self.source_file));
      new_im = im.load()

      width, height = im.size

      # Make sure the height is a multiple of 8
      if height % 8:
         raise ParserException("Pixmap height must be a multiple of 8")

      for y in range(0, height) :
         bloc="// "
         for x in range(0, width) :
            if 0 < new_im[x, y]:
               new_im[x, y] = 1
            bloc += (str(new_im[x, y]))
         self.src.append(bloc)

      self.src.append("const gfx_mono_color_t PROGMEM %s_header[] = {" % self.id)

      for y in range(0, height, 8) :
         for x in range(0, width, 8):
            bloc="   "
            for z in range(8):
               bloc += "0x%.2x%s" % (
                  sum([1<<k if new_im[x+z, y+k] else 0 for k in range(8)]),
                  ',' if z==7 else ', '
               )
            self.src.append(bloc)

      # Remote last ','
      self.src[-1] = self.src[-1][:-1]
      self.src.append("};")
      self.src.append("")

      # Add the bitmap struct
      self.inc.append(f"extern struct gfx_mono_bitmap {self.id}_bm;")

      self.src.append(f"struct gfx_mono_bitmap {self.id}_bm = {{")
      self.src.append(f"   .width = {width},")
      self.src.append(f"   .height = {height},")
      self.src.append("   .type = GFX_MONO_BITMAP_PROGMEM,")
      self.src.append("   {")
      self.src.append(f"      .progmem = {self.id}_header")
      self.src.append("   }")
      self.src.append("};")
      self.src.append("")


class ResourceParser:
   def __init__(self, resource_file):
      self.resources = {}
      self.current_file = None

      # Parse the JSON
      with open(resource_file) as input:
         self.resource_dict = json.load(input)
         parent = Path(resource_file).parent

         # Parse all the resources
         for res in self.resource_dict.get("in", []):
            # Look for the type
            if not isinstance(res, dict):
               raise ParserException("The 'resources' must contain a list of dictionnary")

            type = res.get("type", None)
            if not type:
               raise ParserException("A resource must have a type")
            if not type in ['bitmap']:
               raise ParserException(f"Invalid resource type: '{type}")

            id = res.get("id", None)
            if not id:
               raise ParserException("A resource must have a 'id'")
            if not VALID_ID.match(id):
               raise ParserException(f"Invalid resource id: '{id}")

            if type == "bitmap":
               print(f"Processing bitmap: {id}")
               self.resources[id] = Bitmap(id, parent, res)
               self.resources[id].build(res)

         for files in self.resource_dict.get("out", []):
            # Grab the source
            source = files.get("src", [])
            header = files.get("inc", [])

            # Need at least one
            if not (source and header):
               raise ParserException("Need 'source' and 'header' in 'out'.")

            # Generate the headers for the source and the header
            src = self.get_src_code(source, True)
            inc = self.get_inc_code(header, True)

            # Add optional headers
            src_header = files.get("src_header", [])
            if src_header:
               src.extend(src_header)
               src.append("")

            inc_header = files.get("inc_header", [])
            if inc_header:
               inc.extend(inc_header)
               inc.append("")

            # Iterate the resources
            for res_id in files.get("ids", []):
               # The ID must exists!
               if res_id not in self.resources:
                  raise ParserException(f"Undeclared resource id: {res_id}")

               src.extend(self.resources[res_id].get_code(True))
               inc.extend(self.resources[res_id].get_code())

            # Generate the footers for the source and the header
            src.extend(self.get_src_code(source, False))
            inc.extend(self.get_inc_code(header, False))

            with open(source, "wt") as source_file:
               source_file.writelines('\n'.join(src))

            with open(header, "wt") as header_file:
               header_file.writelines('\n'.join(inc))

   def get_src_code(self, filename, is_header):
      if is_header:
         retval = [
            f"// This file was generated by {sys.argv[0]}. Do not edit.",
         ]
      else:
         retval = ["", """// End of file""",""]

      return retval

   def get_inc_code(self, filename, is_header):
      make_exc = '__'.join(Path(filename).parts).replace('.', '__')
      if is_header:
         retval = [f"#ifndef {make_exc}_was_included"]
         retval.append(f'#define {make_exc}_was_included')
         retval.append('')
         retval.append(f'// This file was generated by {sys.argv[0]}. Do not edit.')
      else:
         retval = ["", f"#endif // ndef {make_exc}_was_included"]
         retval.append('')

      return retval

   def gen_deps(self, depfile):
      out = []
      for files in self.resource_dict.get("out", []):
         out.append(f"{files['src']} {files['inc']}: \\")
         deps = []
         for res_id in files.get("ids", []):
            res = self.resources[res_id]
            deps.append("   " + res.source_file + " \\")
         out.extend(deps)
         out.append("")
         # Append the object requiring the source
         out.append(f"SRCS.rc += {files['src']}")
         out.append("")

      depfile.writelines("\n".join(out))

if __name__ == "__main__":
   import argparse

   parser = argparse.ArgumentParser()

   parser.add_argument(
      '-E', dest='deps', help='Generate a dependency file', type=str, nargs=1)
   parser.add_argument('rcfile')
   args = parser.parse_args()

   try:
      rc = ResourceParser(args.rcfile)
      if args.deps:
         with open(args.deps[0], "wt") as depfile:
            rc.gen_deps(depfile)

   except (FileNotFoundError, ParserException, json.JSONDecodeError) as e:
      print(f"Failed: {e}")
      sys.exit(1)


