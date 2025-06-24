import io
import sys
from pathlib import Path

ops_dir = sys.argv[1]
arr_buf = io.StringIO()
arr_buf.write("const char *names[] = {\n")
with open(file=f'{ops_dir}/opcode.txt', encoding='utf-8') as lines:
    for line in lines:
        opcode = line.strip()
        if not opcode:
            continue
        if opcode.endswith(','):
            opcode = opcode[:-1]
        arr_buf.write(f'        "{opcode}",\n')

buffer = io.StringIO()
buffer.write('#ifndef __OP_GEN_H__\n')
buffer.write('#define __OP_GEN_H__\n')
buffer.write('static inline char *pvm_opcode_name(const uint8_t opcode) {\n')
text = buffer.getvalue()
array_text = arr_buf.getvalue().strip()[:-1]
buffer.write(array_text)
buffer.write("\n};\n")
buffer.write("uint64_t length = sizeof(names)/sizeof(char *);\n")
buffer.write("if(opcode >= length) \n{\n return NULL; \n}\n")
buffer.write("      return names[opcode];\n")
buffer.write("}\n")
buffer.write('#endif')
Path('include/op_gen.h').write_text(buffer.getvalue(), encoding='utf-8')
