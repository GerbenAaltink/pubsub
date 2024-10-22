
#ifndef RETOORII_H
#define RETOORII_H
#include <stdio.h>
#include <stdlib.h>
int retoorii(void * file, int line){
    printf("retoor: %s:%d\n",file,line);
    return NULL;
}
/*#  292 | # define _POSIX_C_SOURCE        200809L*/
#endif

default_header_functions = """
#ifndef RETOORII_H
#define RETOORII_H
#include <stdio.h>
#include <stdlib.h>
int retoorii(void * file, int line){
    printf("retoor: %s:%d\\n",file,line);
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    return NULL;
}
/*#  292 | # define _POSIX_C_SOURCE        200809L*/
#endif
"""

def patchc(file):
    result = [default_header_functions,"\n"]
    with file.open('r') as f:
        lines = f.readlines()
        depth = 0
        skip = 0
        enum = 0
        typedef = 0
        in_multiline_comment = False 
        in_function = False
        in_function = False
        in_comment = False # newline happened, end comment
        for line in lines:
            in_comment = False 
            in_string = False 
            in_single_string = False
            in_brace = False 
            if "enum" in line:
                enum += 1 
                skip += 1
            if "struct" in line:
                typedef += 1
                skip += 1
            prevchar = "" 
            for char in line:
                in_special_character_context = any([in_comment,in_multiline_comment,in_string,in_single_string])
                if not in_special_character_context  and char == "'":
                    in_single_string = True 
                elif in_single_string:
                    if char == "'":
                        in_single_string = False
                if not in_special_character_context and char == '"':
                    in_string = True 
                elif in_string:
                    if char == '"':
                        in_string = False
                    #stays in string
                elif not in_special_character_context and char == "/" and prevchar == "/":
                    in_comment = True
                elif in_comment and char == "\n":
                    in_comment = False 
                elif not in_special_character_context and char == "*" and prevchar == "/":
                    in_multiline_comment = True 
                elif in_multiline_comment and char == "/" and prevchar == "*":
                    in_multiline_comment = False 
                in_special_character_context = any([in_comment,in_multiline_comment,in_string,in_single_string])
                
                prevchar = char
                result.append(char)
                if in_special_character_context:
                    continue
                if char == "{":
                    depth += 1 
                    if depth and (not enum and not typedef):
                        in_function = True
                elif char == "}":
                    if enum:
                        skip -= 1
                        enum -= 1 
                    if typedef:
                        skip -= 1 
                        typedef -= 1
                    depth -= 1
                    if depth <= 0:
                        depth = 0
                        in_function = False 
                        skip = 0
                        typedef = 0
                        enum = 0
                        
                elif char == "(":
                    skip += 1 
                    #in_function = False 
                elif char == ")":
                    skip -=1
                in_function = depth > 0 and skip < 2  and not typedef and not enum and not "typedef" in line and not "}" in line
                if char == "\n" and in_function and not line.strip(" ").endswith(")") \
                    and not line.strip(" ").endswith("=") and not line.strip(" ").endswith("*"): 
                    if not line.strip(" ").startswith("return") and line.strip("\n").endswith(";"):
                        str_indent = " " * (len(line) - len(line.lstrip(" "))) 
                        result.append("{}/* retoorded comment */ retoorii(__FILE__,__LINE__);".format(str_indent))
        return "".join(result)
                  
                  #  else:
                       # print("d:{} s:{} f:{}".format(depth,skip,in_function))
                    
import pathlib

destination_path = pathlib.Path("./patched_debug")
if not destination_path.exists():
    destination_path.mkdir(exist_ok=True,parents=True)

for path in destination_path.glob(".*"):
    print("Skipping, already exists: {}".format(path.absolute()))

count_compiled = 0
for path in pathlib.Path(".").glob("*.*"):

    new_destination = destination_path.joinpath(path.name)
    print("Prepare compile:\n - source: {}\n - destination: {}".format(path.absolute(),new_destination.absolute()))
    if not path.is_file():
        print("Skipping directory {}".format(path.relative_to(".")))
        continue 
    print("Reading {}".format(path.relative_to(".")))
    new_content = patchc(path)

    print("Writing {} bytes to {}".format(len(new_content), new_destination.relative_to(".")))
    with new_destination.open("w") as f:
        f.write(new_content)
    print("Written {} bytes to {}".format(len(new_content),new_destination.relative_to(".")))
    print("File patched: {}".format(new_destination.name))
    #path.write_text(new_content)
    count_compiled += 1

print("Compiled {} files".format(count_compiled))
        
